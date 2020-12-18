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

#include "SndFile.hpp"
#include "Log.hpp"

#include "MACEDecoder.hpp"
#include "IMA4Decoder.hpp"
#include "XLawDecoder.hpp"

#include <iomanip>
#include <utility> // For std::move

const unsigned BUFFER_CMD = 0x8051; // bufferCmd with data offset bit.

namespace
{
    // sampleRate is an unsigned 32-bit fixed-point sample rate:
    // the 16 MSbs are left of the point, and the 16 LSbs are right of the point.
    float fixedSampleRateToFloat(std::uint32_t sampleRate)
    {
        return sampleRate / (16.0f*16.0f*16.0f*16.0f);
    }
}

const unsigned char SndFile::cStandardSoundHeaderEncode = 0x00;
const unsigned char SndFile::cExtendedSoundHeaderEncode = 0xFF;
const unsigned char SndFile::cCompressedSoundHeaderEncode = 0xFE;

std::ostream& operator<<(std::ostream& lhs, const SoundSampleHeader& rhs)
{
    lhs << std::setfill('0'); // For hex values

    lhs <<
        "Base sound sample header: " << std::endl <<
        " -- Sample pointer: " << "0x" << std::setw(8) <<
            std::hex << rhs.samplePtr << std::endl <<

        " -- Length or num. channels: " << std::dec << rhs.lengthOrChannels << std::endl <<
        " -- Total sample area size: " << rhs.sampleArea.size() << std::endl <<
        " -- Sample rate: " << "0x" << std::setw(8) << std::hex <<
            rhs.sampleRate << std::dec << " (" << fixedSampleRateToFloat(rhs.sampleRate) <<
            " Hz)" << std::endl <<

        " -- Loop start: " << "0x" << std::setw(8) << std::hex <<
            rhs.loopStart  << std::endl <<
        " -- Loop end: " << "0x" << std::setw(8) << rhs.loopEnd << std::endl <<
        " -- Encoding: " << "0x" << std::setw(2) <<
            static_cast<unsigned>(rhs.encode) << std::endl <<

        " -- Base frequency: " << "0x" << std::setw(2) << 
            static_cast<unsigned>(rhs.baseFrequency);

    lhs << std::dec << std::setfill(' '); // Restore
    return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const ExtendedSoundSampleHeader& rhs)
{
    lhs << static_cast<const SoundSampleHeader&>(rhs) << std::endl; // Print base class.

    lhs << std::setfill('0'); // For hex values
    lhs <<
        "Extended sound sample header (0xff): " << std::endl <<
        " -- Number of frames: " << rhs.numFrames << std::endl <<

        // Array in big-endian!
        " -- AIFFSampleRate: " << "0x" << std::hex << std::setw(4) << rhs.AIFFSampleRate[0] <<
            std::setw(8) << rhs.AIFFSampleRate[1] << std::setw(8) << rhs.AIFFSampleRate[2] <<
            std::endl <<

        " -- Marker chunk pointer: " << "0x" << std::setw(8) << rhs.markerChunk << std::endl <<
        " -- Instrument chunks pointer: " << "0x" << std::setw(8) <<
            rhs.instrumentChunks << std::endl <<
        " -- AES Recording pointer: " << "0x" << std::setw(8) <<rhs.AESRecording << std::endl <<
        " -- Sample size: " << std::dec << rhs.sampleSize << std::endl <<
        " -- Future use (1): " << "0x" << std::hex << std::setw(4) << rhs.futureUse1 << std::endl <<
        " -- Future use (2): " << "0x" << std::setw(8) << rhs.futureUse2 << std::endl <<
        " -- Future use (3): " << "0x" << std::setw(8) << rhs.futureUse3 << std::endl <<
        " -- Future use (4): " << "0x" << std::setw(8) << rhs.futureUse4;

    lhs << std::dec << std::setfill(' '); // Restore

    return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const CompressedSoundSampleHeader& rhs)
{
    lhs << static_cast<const SoundSampleHeader&>(rhs) << std::endl; // Print base class.

    lhs << std::setfill('0'); // For hex values
    lhs << "Compressed sound sample header (0xfe): " << std::endl <<
        " -- Number of frames: " << rhs.numFrames << std::endl <<

        // Array in big-endian
        " -- AIFFSampleRate: " << "0x" << std::hex << std::setw(4) <<rhs.AIFFSampleRate[0] <<
            std::setw(8) << rhs.AIFFSampleRate[1] << std::setw(8) << rhs.AIFFSampleRate[2] <<
            std::endl <<

        " -- Marker chunk pointer: " << "0x" << std::setw(8) << rhs.markerChunk << std::endl <<
        " -- Format: " << std::string(reinterpret_cast<const char*>(rhs.format), 4) << std::endl <<
        " -- Future use (2): " << "0x" << std::setw(8) <<rhs.futureUse2 << std::endl <<
        " -- State vars pointer: " << "0x" << std::setw(8) <<rhs.stateVars << std::endl <<
        " -- Leftover samples pointer: " << "0x" << std::setw(8) << rhs.leftOverSamples << std::endl <<
        " -- Compression ID: " << std::dec << rhs.compressionID << std::endl <<

        " -- Packet size: " << rhs.packetSize << std::endl <<
        " -- Snth ID: " << "0x" << std::setw(4) <<
            std::hex << rhs.snthID << std::endl <<
        " -- Sample size: " << std::dec << rhs.sampleSize;

    lhs << std::dec << std::setfill(' '); // Restore

    return lhs;
}

// Creates appropriate decoder using the currently loaded header information.
void CompressedSoundSampleHeader::createDecoder()
{
    std::string formatString =
        std::string(reinterpret_cast<const char*>(this->format), 4);

    if(this->compressionID == 3 ||
       formatString == "mac3" ||
       formatString == "MAC3")
    {
        this->decoder = std::unique_ptr<Decoder>(new MACEDecoder());
    } else if(formatString == "ima4" || formatString == "IMA4")
    {
        this->decoder = std::unique_ptr<Decoder>(new IMA4Decoder());
    } else if(formatString == "alaw" || formatString == "ALAW")
    {
        this->decoder = std::unique_ptr<Decoder>(new ALawDecoder());
    } else if(formatString == "ulaw" || formatString == "ULAW")
    {
        this->decoder = std::unique_ptr<Decoder>(new ULawDecoder());
    } else if(this->compressionID == 0)
    {
        // Uncompressed sound using the compressed sound header;
        // no need to create a decoder.
    } else
    {
        Log::err << "Error: cannot create decoder! " <<
            "Unsupported compression format: '" << formatString << "' (ID: " <<
            this->compressionID << ")." << std::endl;
    }
}

SndFile::SndFile(std::istream& file, const std::string& fileName)
    : mFileName(fileName)
    , mFile(file)
{
    parse();
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
        doBufferCommand(fullBufferCmd);
    else
    {
        Log::err << "Error: bufferCmd not found in snd file! Cannot convert." << std::endl;
        return false;
    }

    return true;
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

const SoundSampleHeader& SndFile::getSoundSampleHeader() const
{
    return *mSoundSampleHeader;
}

// Static.
// Reads sound sample data from current position in stream.
// sampleLength in bytes.
void SndFile::readSoundSampleData(std::istream& stream, std::vector<std::uint8_t>& buffer,
        std::size_t sampleLength, unsigned bytesPerValue)
{
    // In bytes, since we are always dealing with a uint8_t vector.
    buffer.resize(sampleLength);

    // Snd only supports 8-bit or 16-bit samples (I think).
    if(bytesPerValue == 1)
    {
        readBigArray(stream, buffer.data(), sampleLength);
    } else if(bytesPerValue == 2)
    {
        // If we have 2 bytes per sample, we must make sure we convert
        // them to native endianness!
        // Note: 16-bit samples are normally signed, but that doesn't
        // change anything here.
        readBigArray(stream,
            reinterpret_cast<std::uint16_t*>(buffer.data()),
            sampleLength / bytesPerValue);
    }
}

// Offset from beginning of file, must be in native endianness.
// Returns true on success, false on failure.
bool SndFile::loadSoundSampleHeader(std::uint16_t offset)
{
    // All headers are derived from the standard header.
    std::unique_ptr<SoundSampleHeader> standardHeader(new SoundSampleHeader());
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
        // Standard sound files always have 1 byte/sample.
        readSoundSampleData(mFile, standardHeader->sampleArea,
            standardHeader->lengthOrChannels, 1);

        // Debugging info.
        Log::verb << *standardHeader << std::endl;

        mSoundSampleHeader = std::move(standardHeader);
        return true;
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

        // Copy sample data.
        // Length (bytes) = numFrames * sampleSize/8 * number of channels (lengthOrChannels)
        std::size_t sampleLength =
            extendedHeader->numFrames *
            extendedHeader->sampleSize/8 *
            extendedHeader->lengthOrChannels;

        readSoundSampleData(mFile, extendedHeader->sampleArea, sampleLength,
            extendedHeader->sampleSize/8);

        // Debug info.
        Log::verb << *extendedHeader << std::endl;

        mSoundSampleHeader = std::move(extendedHeader);
        return true;
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

        compressedHeader->createDecoder();

        // Copy sample data.
        // numFrames is the number of packet frames, not sample frames.
        // So, numFrames * numChannels = numPackets.
        std::size_t sampleLength = compressedHeader->decoder->getEncodedSize(
                compressedHeader->numFrames * compressedHeader->lengthOrChannels
        );

        // Read values as bytes, since we only figure out endianness when
        // decoding compressed data.
        readSoundSampleData(mFile, compressedHeader->sampleArea, sampleLength, 1);

        // Debug info.
        Log::verb << *compressedHeader << std::endl;

        mSoundSampleHeader = std::move(compressedHeader);
        return true;
    }

    Log::err << "Error: unrecognized sound sampler header encoding! Cannot convert." <<
        std::endl;
    mSoundSampleHeader = std::move(standardHeader);
    return false;
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
