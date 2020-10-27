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
#include <iostream>
#include <iomanip>

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
        " -- Real sample length: " << rhs.samples.size() << std::endl <<
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
        " -- Instrument chunks pointer: " << "0x" << std::setw(8) << rhs.instrumentChunks << std::endl <<
        " -- AES Recording pointer: " << "0x" << std::setw(8) <<rhs.AESRecording << std::endl <<
        " -- Sample size: " << "0x" << std::setw(4) << rhs.sampleSize << std::endl <<
        " -- Future use (1): " << "0x" << std::setw(4) << rhs.futureUse1 << std::endl <<
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

        // Array in big-endian!
        " -- AIFFSampleRate: " << "0x" << std::hex << std::setw(4) <<rhs.AIFFSampleRate[0] <<
            std::setw(8) << rhs.AIFFSampleRate[1] << std::setw(8) << rhs.AIFFSampleRate[2] <<
            std::endl <<

        " -- Marker chunk pointer: " << "0x" << std::setw(8) << rhs.markerChunk << std::endl <<
        " -- Format: " << "0x" << std::setw(8) << rhs.format << std::endl <<
        " -- Future use (2): " << "0x" << std::setw(8) <<rhs.futureUse2 << std::endl <<
        " -- State vars pointer: " << "0x" << std::setw(8) <<rhs.stateVars << std::endl <<
        " -- Leftover samples pointer: " << "0x" << std::setw(8) << rhs.leftOverSamples << std::endl <<
        " -- Compression ID: " << "0x" << std::setw(4) << rhs.compressionID << std::endl <<

        " -- Packet size: " << std::dec << rhs.packetSize << std::endl <<
        " -- Snth ID: " << "0x" << std::setw(4) <<
            std::hex << rhs.snthID << std::endl <<
        " -- Sample size: " << std::dec << rhs.sampleSize;

    lhs << std::dec << std::setfill(' '); // Restore

    return lhs;
}

SndFile::SndFile()
{
}

SndFile::SndFile(const std::string& sndFileName)
    : mFileName(sndFileName)
{
    open(sndFileName);
}

// Parse the current snd file.
// Returns true on success, false on failure.
bool SndFile::parse()
{
    if(mFile.fail())
    {
        std::cerr << "Error: file '" << mFileName << "' cannot be parsed, since " <<
            "it is invalid!" << std::endl;
        return false;
    }

    mFile.seekg(0, mFile.beg);

    mFormat = readValueFromFile<decltype(mFormat)>(mFile);
    mNumDataFormats = readValueFromFile<decltype(mNumDataFormats)>(mFile);
    
    if(mNumDataFormats == 0)
    {
        std::cerr << "Error: snd file contains 0 data formats!" << std::endl;
        return false;
    }

    mFirstDataFormatID = readValueFromFile<decltype(mFirstDataFormatID)>(mFile);
    mInitOptionForChannel = readValueFromFile<decltype(mInitOptionForChannel)>(mFile);
    mNumSoundCommands = readValueFromFile<decltype(mNumSoundCommands)>(mFile);

    for(std::size_t i = 0; i < mNumSoundCommands; ++i)
    {
        std::uint64_t command = readValueFromFile<decltype(command)>(mFile);
        mSoundCommands.push_back(command);
    }

    // Warn if more than 1 command.
    if(mNumSoundCommands > 1)
    {
        std::cout << "Warning: more than 1 sound command found in 'snd ' file! " <<
            "May not convert correctly. (Are you sure your 'snd ' file only contains " <<
            "a single sound sample?)" << std::endl;
    }

    // Immediately interpret bufferCmd if present.
    // We only interpret the first bufferCmd, so watchout if there is more than one!
    std::uint64_t fullBufferCmd = findSoundCommand(BUFFER_CMD);

    if(fullBufferCmd != 0)
        doBufferCommand(fullBufferCmd);
    else
    {
        std::cerr << "Error: bufferCmd not found in snd file! Cannot convert." << std::endl;
        return false;
    }
}

// Finds first instance of cmdName, and returns entire command.
// Returns 0 on failure.
std::uint64_t SndFile::findSoundCommand(std::uint16_t cmdName)
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
    std::uint16_t param1 = static_cast<std::uint16_t>(command >> 32);
    std::uint16_t param2 = static_cast<std::uint16_t>(command);

    if(cmdName != BUFFER_CMD)
    {
        std::cerr << "Error: not a buffer command! Cannot interpret command." <<
            std::endl;
        return false;
    }

    if(mFile.fail())
    {
        std::cerr << "Error: cannot interpret bufferCmd! File is invalid." <<
            std::endl;
        return false;
    }

    mSoundSampleHeader = readSoundSampleHeader(param2);
}

// Opens and parses file.
// Returns true on success, false on failure.
bool SndFile::open(const std::string& sndFileName)
{
    mFile.open(sndFileName);

    if(mFile.fail())
    {
        std::cerr << "Error: could not open file '" << sndFileName << "'!" <<
            std::endl;
        return false;
    }

    return parse();
}

const SoundSampleHeader& SndFile::getSoundSampleHeader() const
{
    return *mSoundSampleHeader;
}

// Offset from beginning of file, must be in native endianness.
std::unique_ptr<SoundSampleHeader> SndFile::readSoundSampleHeader(std::uint16_t offset)
{
    std::unique_ptr<SoundSampleHeader> standardHeader(new SoundSampleHeader());
    mFile.seekg(offset, mFile.beg);

    standardHeader->samplePtr = readValueFromFile<decltype(standardHeader->samplePtr)>(mFile);
    standardHeader->lengthOrChannels = readValueFromFile<decltype(standardHeader->lengthOrChannels)>(mFile);
    standardHeader->sampleRate = readValueFromFile<decltype(standardHeader->sampleRate)>(mFile);
    standardHeader->loopStart = readValueFromFile<decltype(standardHeader->loopStart)>(mFile);
    standardHeader->loopEnd = readValueFromFile<decltype(standardHeader->loopEnd)>(mFile);
    standardHeader->encode = readValueFromFile<decltype(standardHeader->encode)>(mFile);
    standardHeader->baseFrequency = readValueFromFile<decltype(standardHeader->baseFrequency)>(mFile);

    if(standardHeader->encode == cStandardSoundHeaderEncode)
    {
        // Copy sample data.
        standardHeader->samples.resize(standardHeader->lengthOrChannels);
        // Byte order is conserved.
        mFile.read(reinterpret_cast<char*>(standardHeader->samples.data()), standardHeader->lengthOrChannels);

        // Debugging info.
        std::cout << *standardHeader << std::endl;

        // Return standard header.
        return standardHeader;
    } else if(standardHeader->encode == cExtendedSoundHeaderEncode)
    {
        // Extended sound header.
        // Copy values already read:
        std::unique_ptr<ExtendedSoundSampleHeader> extendedHeader(
            new ExtendedSoundSampleHeader(*standardHeader)
        );

        extendedHeader->numFrames = readValueFromFile<decltype(extendedHeader->numFrames)>(mFile);
        extendedHeader->AIFFSampleRate[0] = readValueFromFile<std::uint32_t>(mFile, 2);
        extendedHeader->AIFFSampleRate[1] = readValueFromFile<std::uint32_t>(mFile);
        extendedHeader->AIFFSampleRate[2] = readValueFromFile<std::uint32_t>(mFile);
        extendedHeader->markerChunk = readValueFromFile<decltype(extendedHeader->markerChunk)>(mFile);
        extendedHeader->instrumentChunks = readValueFromFile<decltype(extendedHeader->instrumentChunks)>(mFile);
        extendedHeader->AESRecording = readValueFromFile<decltype(extendedHeader->AESRecording)>(mFile);
        extendedHeader->sampleSize = readValueFromFile<decltype(extendedHeader->sampleSize)>(mFile);
        extendedHeader->futureUse1 = readValueFromFile<decltype(extendedHeader->futureUse1)>(mFile);
        extendedHeader->futureUse2 = readValueFromFile<decltype(extendedHeader->futureUse2)>(mFile);
        extendedHeader->futureUse3 = readValueFromFile<decltype(extendedHeader->futureUse3)>(mFile);
        extendedHeader->futureUse4 = readValueFromFile<decltype(extendedHeader->futureUse4)>(mFile);

        // Copy sample data.
        // Length = numFrames * number of channels (lengthOrChannels)
        extendedHeader->samples.resize(extendedHeader->numFrames * extendedHeader->lengthOrChannels);
        mFile.read(reinterpret_cast<char*>(extendedHeader->samples.data()),
                extendedHeader->numFrames * extendedHeader->lengthOrChannels);

        // Debug info.
        std::cout << *extendedHeader << std::endl;

        // Return extended header.
        return extendedHeader;
    } else if(standardHeader->encode == cCompressedSoundHeaderEncode)
    {
        // Compressed sound header
        // Copy values already read:
        std::unique_ptr<CompressedSoundSampleHeader> compressedHeader(
            new CompressedSoundSampleHeader(*standardHeader)
        );

        compressedHeader->numFrames = readValueFromFile<decltype(compressedHeader->numFrames)>(mFile);
        compressedHeader->AIFFSampleRate[0] = readValueFromFile<std::uint32_t>(mFile, 2);
        compressedHeader->AIFFSampleRate[1] = readValueFromFile<std::uint32_t>(mFile);
        compressedHeader->AIFFSampleRate[2] = readValueFromFile<std::uint32_t>(mFile);
        compressedHeader->markerChunk = readValueFromFile<decltype(compressedHeader->markerChunk)>(mFile);
        compressedHeader->format = readValueFromFile<decltype(compressedHeader->format)>(mFile);
        compressedHeader->futureUse2 = readValueFromFile<decltype(compressedHeader->futureUse2)>(mFile);
        compressedHeader->stateVars = readValueFromFile<decltype(compressedHeader->stateVars)>(mFile);
        compressedHeader->leftOverSamples = readValueFromFile<decltype(compressedHeader->leftOverSamples)>(mFile);
        compressedHeader->compressionID = readValueFromFile<decltype(compressedHeader->compressionID)>(mFile);
        compressedHeader->packetSize = readValueFromFile<decltype(compressedHeader->packetSize)>(mFile);
        compressedHeader->snthID = readValueFromFile<decltype(compressedHeader->snthID)>(mFile);
        compressedHeader->sampleSize = readValueFromFile<decltype(compressedHeader->sampleSize)>(mFile);

        // Copy sample data.
        // Length = numFrames * number of channels (lengthOrChannels)
        compressedHeader->samples.resize(compressedHeader->numFrames * compressedHeader->lengthOrChannels);
        mFile.read(reinterpret_cast<char*>(compressedHeader->samples.data()),
                compressedHeader->numFrames * compressedHeader->lengthOrChannels);

        // Debug info.
        std::cout << *compressedHeader << std::endl;

        // Return compressed header.
        return compressedHeader;
    }

    std::cerr << "Error: unrecognized sound sampler header encoding! Cannot convert." <<
        std::endl;
    return std::unique_ptr<SoundSampleHeader>(new SoundSampleHeader());
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

        " -- Number of sound commands: " << std::dec << rhs.mNumSoundCommands << std::endl <<

        " -- First sound command: " << "0x" << std::setw(16) << 
            std::hex << rhs.mSoundCommands[0];;

    lhs << std::dec << std::setfill(' '); // Restore

    return lhs;
}
