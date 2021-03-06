// Copyright 2020 Carl Hewett
//
// This file is part of SndToWAV.
//
// SndToWAV is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SndToWAV is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SndToWAV. If not, see <http://www.gnu.org/licenses/>.

// Reference:
// https://developer.apple.com/library/archive/documentation/mac/pdf/Sound/Sound_Manager.pdf

#include "SndFile.hpp"
#include "Log.hpp"

#include "NullDecoder.hpp"
#include "MACEDecoder.hpp"
#include "IMA4Decoder.hpp"
#include "XLawDecoder.hpp"

#include <iomanip>
#include <utility> // For std::move
#include <stdexcept>

namespace
{
    const unsigned BUFFER_CMD = 0x8051; // bufferCmd with data offset bit.
}

const std::uint8_t SndFile::cStandardSoundHeaderEncode = 0x00;
const std::uint8_t SndFile::cExtendedSoundHeaderEncode = 0xFF;
const std::uint8_t SndFile::cCompressedSoundHeaderEncode = 0xFE;

SndFile::SndFile(std::istream& file, const std::string& fileName)
    : mFileName(fileName)
    , mFile(file)
{
    parse();
    decode();
}

// Parse the current snd file.
// Returns true on success, false on failure.
bool SndFile::parse()
{
    if(mFile.fail())
    {
        Log::err << "Error: file '" << mFileName << "' cannot be parsed, since " <<
            "it is invalid!" << std::endl;
        return false;
    }

    mFile.seekg(0, mFile.beg);

    mFormat = readBigValue<decltype(mFormat)>(mFile);
    mNumDataFormats = readBigValue<decltype(mNumDataFormats)>(mFile);
    
    if(mNumDataFormats == 0)
    {
        Log::err << "Error: snd file contains 0 data formats!" << std::endl;
        return false;
    }

    mFirstDataFormatID = readBigValue<decltype(mFirstDataFormatID)>(mFile);
    mInitOptionForChannel = readBigValue<decltype(mInitOptionForChannel)>(mFile);
    mNumSoundCommands = readBigValue<decltype(mNumSoundCommands)>(mFile);

    for(std::size_t i = 0; i < mNumSoundCommands; ++i)
    {
        std::uint64_t command = readBigValue<decltype(command)>(mFile);
        mSoundCommands.push_back(command);
    }

    // Warn if more than 1 command.
    if(mNumSoundCommands > 1)
    {
        Log::warn << "Warning: more than 1 sound command found in 'snd ' file! " <<
            "May not convert correctly. (Are you sure your 'snd ' file only contains " <<
            "a single sound sample?)" << std::endl;
    }

    // Print our snd file info for debug.
    Log::verb << *this << std::endl;

    // Immediately interpret bufferCmd if present.
    // We only interpret the first bufferCmd, so watchout if there is more than one!
    std::uint64_t fullBufferCmd = findSoundCommand(BUFFER_CMD);
    if(fullBufferCmd != 0)
    {
        return doBufferCommand(fullBufferCmd);
    }

    Log::err << "Error: bufferCmd not found in snd file! Cannot convert." << std::endl;
    return false;
}

// Call after parsing!
bool SndFile::decode()
{
    if(mDecoder == nullptr)
    {
        Log::err << "Error: decoder was not created! Cannot decode." << std::endl;
        return false;
    }

    if(mSoundSampleHeader == nullptr)
    {
        Log::err << "Error: sound sample header is not loaded! Cannot decode." <<
            std::endl;
        return false;
    }

    // Decode!
    if(mSoundSampleHeader->encode == cStandardSoundHeaderEncode)
    {
        // For basic sounds, we don't have a number of channels; it is always 1.
        return mDecoder->decode(mSoundSampleHeader->sampleArea, 1);
    }

    return mDecoder->decode(
        mSoundSampleHeader->sampleArea,
        mSoundSampleHeader->lengthOrChannels
    );
}

// Finds first instance of cmdName, and returns entire command.
// Returns 0 on failure.
std::uint64_t SndFile::findSoundCommand(std::uint16_t cmdName) const
{
    for(std::uint64_t command : mSoundCommands)
    {
        if((command >> 48) == cmdName)
            return command;
    }

    return 0;
}

// Interpret a bufferCmd.
// Make sure command already has native endianness!
// Fills sound sample header and sound data.
// Returns true on success, false on failure.
bool SndFile::doBufferCommand(std::uint64_t command)
{
    // Truncate MSbs.
    // (Already in native endianness.)
    std::uint16_t cmdName = static_cast<std::uint16_t>(command >> 48);
    // First param (unused): param1 = static_cast<std::uint16_t>(command >> 32);
    std::uint16_t param2 = static_cast<std::uint16_t>(command);

    if(cmdName != BUFFER_CMD)
    {
        Log::err << "Error: not a buffer command! Cannot interpret command." <<
            std::endl;
        return false;
    }

    if(mFile.fail())
    {
        Log::err << "Error: cannot interpret bufferCmd! File is invalid." <<
            std::endl;
        return false;
    }

    loadSoundSampleHeader(param2);
    return true;
}

// Offset from beginning of file, must be in native endianness.
// Returns true on success, false on failure.
bool SndFile::loadSoundSampleHeader(std::size_t offset)
{
    // All headers are derived from the standard header.
    std::unique_ptr<SoundSampleHeader> standardHeader(new SoundSampleHeader());
    std::size_t sampleDataSize = 0; // In bytes.

    mFile.seekg(offset, mFile.beg);

    standardHeader->samplePtr = readBigValue<decltype(standardHeader->samplePtr)>(mFile);
    standardHeader->lengthOrChannels = readBigValue<decltype(standardHeader->lengthOrChannels)>(mFile);
    standardHeader->sampleRate = readBigValue<decltype(standardHeader->sampleRate)>(mFile);
    standardHeader->loopStart = readBigValue<decltype(standardHeader->loopStart)>(mFile);
    standardHeader->loopEnd = readBigValue<decltype(standardHeader->loopEnd)>(mFile);
    standardHeader->encode = readBigValue<decltype(standardHeader->encode)>(mFile);
    standardHeader->baseFrequency = readBigValue<decltype(standardHeader->baseFrequency)>(mFile);

    // Checks
    if(standardHeader->samplePtr != 0)
    {
        Log::err << "Error: sound sample data pointer is not null! Cannot read data." <<
            std::endl;
        return true;
    }

    if(standardHeader->encode == cStandardSoundHeaderEncode)
    {
        // Standard header sounds are always 8-bit.
        mDecoder = std::unique_ptr<Decoder>(new NullDecoder(8));
        sampleDataSize = standardHeader->lengthOrChannels;

        mSoundSampleHeader = std::move(standardHeader);
    } else if(standardHeader->encode == cExtendedSoundHeaderEncode)
    {
        // Extended sound header.
        // Copy values already read:
        std::unique_ptr<ExtendedSoundSampleHeader> extendedHeader(
            new ExtendedSoundSampleHeader(*standardHeader)
        );

        extendedHeader->numFrames = readBigValue<decltype(extendedHeader->numFrames)>(mFile);
        extendedHeader->AIFFSampleRate[0] = readBigValue<std::uint32_t>(mFile, 2);
        extendedHeader->AIFFSampleRate[1] = readBigValue<std::uint32_t>(mFile);
        extendedHeader->AIFFSampleRate[2] = readBigValue<std::uint32_t>(mFile);
        extendedHeader->markerChunk = readBigValue<decltype(extendedHeader->markerChunk)>(mFile);
        extendedHeader->instrumentChunks = readBigValue<decltype(extendedHeader->instrumentChunks)>(mFile);
        extendedHeader->AESRecording = readBigValue<decltype(extendedHeader->AESRecording)>(mFile);
        extendedHeader->sampleSize = readBigValue<decltype(extendedHeader->sampleSize)>(mFile);
        extendedHeader->futureUse1 = readBigValue<decltype(extendedHeader->futureUse1)>(mFile);
        extendedHeader->futureUse2 = readBigValue<decltype(extendedHeader->futureUse2)>(mFile);
        extendedHeader->futureUse3 = readBigValue<decltype(extendedHeader->futureUse3)>(mFile);
        extendedHeader->futureUse4 = readBigValue<decltype(extendedHeader->futureUse4)>(mFile);

        // Create decoder.
        mDecoder = std::unique_ptr<Decoder>(new NullDecoder(extendedHeader->sampleSize));

        // numPackets = numSamples = numFrames * numChannels.
        sampleDataSize = mDecoder->getEncodedSize(
                extendedHeader->numFrames * extendedHeader->lengthOrChannels
        );

        mSoundSampleHeader = std::move(extendedHeader);
    } else if(standardHeader->encode == cCompressedSoundHeaderEncode)
    {
        // Compressed sound header
        // Copy values already read:
        std::unique_ptr<CompressedSoundSampleHeader> compressedHeader(
            new CompressedSoundSampleHeader(*standardHeader)
        );

        compressedHeader->numFrames = readBigValue<decltype(compressedHeader->numFrames)>(mFile);
        compressedHeader->AIFFSampleRate[0] = readBigValue<std::uint32_t>(mFile, 2);
        compressedHeader->AIFFSampleRate[1] = readBigValue<std::uint32_t>(mFile);
        compressedHeader->AIFFSampleRate[2] = readBigValue<std::uint32_t>(mFile);
        compressedHeader->markerChunk = readBigValue<decltype(compressedHeader->markerChunk)>(mFile);
        readBigArray<std::uint8_t>(mFile, compressedHeader->format, 4);
        compressedHeader->futureUse2 = readBigValue<decltype(compressedHeader->futureUse2)>(mFile);
        compressedHeader->stateVars = readBigValue<decltype(compressedHeader->stateVars)>(mFile);
        compressedHeader->leftOverSamples = readBigValue<decltype(compressedHeader->leftOverSamples)>(mFile);
        compressedHeader->compressionID = readBigValue<decltype(compressedHeader->compressionID)>(mFile);
        compressedHeader->packetSize = readBigValue<decltype(compressedHeader->packetSize)>(mFile);
        compressedHeader->snthID = readBigValue<decltype(compressedHeader->snthID)>(mFile);
        compressedHeader->sampleSize = readBigValue<decltype(compressedHeader->sampleSize)>(mFile);

        // Create decoder.
        if(compressedHeader->compressionID == 0)
        {
            // Uncompressed sound using compressed sound header.
            mDecoder = std::unique_ptr<Decoder>(new NullDecoder(compressedHeader->sampleSize));
        } else
        {
            std::string formatString =
                std::string(reinterpret_cast<const char*>(compressedHeader->format), 4);
            createDecompressionDecoder(formatString, compressedHeader->compressionID);
        }

        // numFrames is the number of packet frames, not sample frames.
        // So, numFrames * numChannels = numPackets.
        sampleDataSize = mDecoder->getEncodedSize(
                compressedHeader->numFrames * compressedHeader->lengthOrChannels
        );

        mSoundSampleHeader = std::move(compressedHeader);
    } else
    {
        Log::err << "Error: unrecognized sound sampler header encoding! Cannot convert." <<
            std::endl;
        mSoundSampleHeader = std::move(standardHeader);
        return false;
    }

    // Load sample data.
    loadSampleData(mFile.tellg(), sampleDataSize);

    // Debugging info.
    Log::verb << *mSoundSampleHeader << std::endl;

    return true;
}

// Offset is from beginning of file.
// Sample data length in bytes.
void SndFile::loadSampleData(std::size_t offset, std::size_t sampleDataLength)
{
    mFile.seekg(offset, mFile.beg);

    // Resize sample area.
    mSoundSampleHeader->sampleArea.resize(sampleDataLength);
    readBigArray<std::uint8_t>(
        mFile,
        mSoundSampleHeader->sampleArea.data(),
        sampleDataLength
    );
}

void SndFile::createDecompressionDecoder(const std::string& formatString,
    int compressionID)
{
    if(compressionID == 3 ||
       formatString == "mac3" ||
       formatString == "MAC3")
    {
        mDecoder = std::unique_ptr<Decoder>(new MACEDecoder());
    } else if(formatString == "ima4" || formatString == "IMA4")
    {
        mDecoder = std::unique_ptr<Decoder>(new IMA4Decoder());
    } else if(formatString == "alaw" || formatString == "ALAW")
    {
        mDecoder = std::unique_ptr<Decoder>(new ALawDecoder());
    } else if(formatString == "ulaw" || formatString == "ULAW")
    {
        mDecoder = std::unique_ptr<Decoder>(new ULawDecoder());
    } else
    {
        Log::err << "Error: cannot create decoder! " <<
            "Unsupported compression format: '" << formatString <<
            "' (ID: " << compressionID << ")." << std::endl;
    }
}

const SoundSampleHeader& SndFile::getSoundSampleHeader() const
{
    return *mSoundSampleHeader;
}

const Decoder& SndFile::getDecoder() const
{
    if(mDecoder == nullptr)
    {
        Log::err << "Error: cannot get decoder! It does not exist." << std::endl;
        throw std::logic_error("Cannot get decoder; it does not exist.");
    }

    return *mDecoder;
}

// Print parsed data, for debugging.
std::ostream& operator<<(std::ostream& lhs, const SndFile& rhs)
{
    lhs << std::setfill('0'); // For hex values

    lhs <<
        "Snd file '" << rhs.mFileName << "' header:" << std::endl <<
        " -- File format: " << rhs.mFormat << std::endl <<
        " -- Number of data formats: " << rhs.mNumDataFormats << std::endl <<
        " -- First data format ID: " << rhs.mFirstDataFormatID << std::endl <<

        " -- Init option for channel: " << "0x" << std::setw(8) <<
            std::hex << rhs.mInitOptionForChannel << std::endl <<

        " -- Number of sound commands: " << std::dec << rhs.mNumSoundCommands;

        if(rhs.mSoundCommands.size() != 0)
        {
            lhs << std::endl <<
                " -- First sound command: " << "0x" << std::setw(16) <<
                std::hex << rhs.mSoundCommands[0];
        }

    lhs << std::dec << std::setfill(' '); // Restore

    return lhs;
}

