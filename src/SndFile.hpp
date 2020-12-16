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

#ifndef SND_FILE_HPP
#define SND_FILE_HPP

#include "SndToWAV.hpp" // For endian stuff

#include <string>
#include <istream>
#include <ostream>
#include <cstdint> // Fixed-width types
#include <cstddef> // For std::size_t
#include <vector>
#include <memory>

class SoundSampleHeader
{
public:
    virtual ~SoundSampleHeader() = default;

    std::uint32_t samplePtr = 0;
    std::int32_t lengthOrChannels = 0; // Number of samples or number of channels (for compressed)
    std::uint32_t sampleRate = 0;
    std::int32_t loopStart = 0;
    std::int32_t loopEnd = 0;
    std::uint8_t encode = 0;
    std::uint8_t baseFrequency = 0;

    std::vector<std::uint8_t> sampleArea; // This is in the header according to docs...

    friend std::ostream& operator<<(std::ostream& lhs, const SoundSampleHeader& rhs);
};

class ExtendedSoundSampleHeader : public SoundSampleHeader
{
public:
    ExtendedSoundSampleHeader() = default;
    ExtendedSoundSampleHeader(const SoundSampleHeader& soundSampleHeader)
        : SoundSampleHeader(soundSampleHeader) {};

    std::int32_t numFrames = 0;
    // 80-bit extended value.
    // Array ordered in big-endian, but each cell is in native endianness.
    // AIFFSampleRate is essentially the same value as the standard header
    // sampleRate.
    std::uint32_t AIFFSampleRate[2] = {0};
    std::uint32_t markerChunk = 0;
    std::uint32_t instrumentChunks = 0;
    std::uint32_t AESRecording = 0;
    std::int16_t sampleSize = 0;
    std::int16_t futureUse1 = 0;
    std::uint32_t futureUse2 = 0;
    std::uint32_t futureUse3 = 0;
    std::uint32_t futureUse4 = 0;

    friend std::ostream& operator<<(std::ostream& lhs, const ExtendedSoundSampleHeader& rhs);
};

class CompressedSoundSampleHeader : public SoundSampleHeader
{
public:
    CompressedSoundSampleHeader() = default;
    CompressedSoundSampleHeader(const SoundSampleHeader& soundSampleHeader)
        : SoundSampleHeader(soundSampleHeader) {};

    std::int32_t numFrames = 0;
    // 80-bit extended value.
    // Array ordered in big-endian, but each cell is in native endianness.
    // AIFFSampleRate is essentially the same value as the standard header
    // sampleRate.
    std::uint32_t AIFFSampleRate[2] = {0};
    std::uint32_t markerChunk = 0;
    std::uint8_t format[4] = {0}; // 4-char string.
    std::int32_t futureUse2 = 0;
    std::uint32_t stateVars = 0;
    std::uint32_t leftOverSamples = 0;

    std::int16_t compressionID = 0;
    std::int16_t packetSize = 0;
    std::int16_t snthID = 0;
    std::int16_t sampleSize = 0;

    friend std::ostream& operator<<(std::ostream& lhs, const CompressedSoundSampleHeader& rhs);
};

class SndFile
{
private:

    // Safe endian.
    // bigStream is a Big-endian input stream.
    template<class T>
    static T readBigValue(std::istream& bigStream)
    {
        T readBigValue;
        bigStream.read(reinterpret_cast<char*>(&readBigValue), sizeof(T));
        return SndToWAV::makeBigEndianNative(readBigValue);
    }

    // Safe endian.
    // bigStream is a Big-endian input stream.
    // Only reads length bytes as LSBs, fills rest (MSBs) with 0s.
    template<class T>
    static T readBigValue(std::istream& bigStream, std::size_t length)
    {
        T readBigValue = 0; // Will with 0s.
        // Fill data in the LSBs of readValue (as Big-endian).
        bigStream.read(reinterpret_cast<char*>(&readBigValue) + (sizeof(T)-length), length);
        return SndToWAV::makeBigEndianNative(readBigValue);
    }

    // Safe endian.
    // bigStream is a Big-endian input stream.
    template<class T>
    static void readBigArray(std::istream& bigStream, T* buffer, std::size_t length)
    {
        for(std::size_t i = 0; i < length; ++i)
        {
            bigStream.read(reinterpret_cast<char*>(&buffer[i]), sizeof(T));
            buffer[i] = SndToWAV::makeBigEndianNative(buffer[i]);
        }
    }

    std::string mFileName;
    std::istream& mFile;
    
    std::uint16_t mFormat;
    std::uint16_t mNumDataFormats;
    std::uint16_t mFirstDataFormatID;
    std::uint32_t mInitOptionForChannel;
    std::uint16_t mNumSoundCommands;
    std::vector<std::uint64_t> mSoundCommands;

    std::unique_ptr<SoundSampleHeader> mSoundSampleHeader; // Polymorphic

    std::vector<std::uint64_t> mSoundData; // Filled when interpreting bufferCmd.

    bool parse();
    std::uint64_t findSoundCommand(std::uint16_t cmdName) const;
    bool doBufferCommand(std::uint64_t command);

    static void readSoundSampleData(std::istream& stream, std::vector<std::uint8_t>& buffer,
        std::size_t sampleLength, unsigned bytesPerSample);

    std::unique_ptr<SoundSampleHeader> readSoundSampleHeader(std::uint16_t offset);

    friend std::ostream& operator<<(std::ostream& lhs, const SndFile& rhs);

public:
    static const unsigned char cStandardSoundHeaderEncode;
    static const unsigned char cExtendedSoundHeaderEncode;
    static const unsigned char cCompressedSoundHeaderEncode;

    SndFile(std::istream& file, const std::string& fileName);

    const SoundSampleHeader& getSoundSampleHeader() const;
};

#endif // SND_FILE_HPP
