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

#ifndef SOUND_SAMPLE_HEADER_HPP
#define SOUND_SAMPLE_HEADER_HPP

#include <cstdint>
#include <vector>
#include <ostream>

class SoundSampleHeader
{
public:
    virtual ~SoundSampleHeader() = default;
    virtual void print(std::ostream& stream) const;

    std::uint32_t samplePtr = 0;
    std::int32_t lengthOrChannels = 0; // Number of samples or number of channels (for compressed)
    std::uint32_t sampleRate = 0;
    std::int32_t loopStart = 0;
    std::int32_t loopEnd = 0;
    std::uint8_t encode = 0;
    std::uint8_t baseFrequency = 0;

    std::vector<std::uint8_t> sampleArea; // This is in the header according to docs...
};

class ExtendedSoundSampleHeader : public SoundSampleHeader
{
public:
    ExtendedSoundSampleHeader() = default;
    ExtendedSoundSampleHeader(const SoundSampleHeader& soundSampleHeader)
        : SoundSampleHeader(soundSampleHeader) {};

    virtual void print(std::ostream& stream) const;

    std::int32_t numFrames = 0;
    // 80-bit extended value.
    // Array ordered in big-endian, but each cell is in native endianness.
    // AIFFSampleRate is essentially the same value as the standard header
    // sampleRate.
    std::uint32_t AIFFSampleRate[3] = {0};
    std::uint32_t markerChunk = 0;
    std::uint32_t instrumentChunks = 0;
    std::uint32_t AESRecording = 0;
    std::int16_t sampleSize = 0;
    std::int16_t futureUse1 = 0;
    std::uint32_t futureUse2 = 0;
    std::uint32_t futureUse3 = 0;
    std::uint32_t futureUse4 = 0;
};

class CompressedSoundSampleHeader : public SoundSampleHeader
{
public:
    CompressedSoundSampleHeader() = default;
    CompressedSoundSampleHeader(const SoundSampleHeader& soundSampleHeader)
        : SoundSampleHeader(soundSampleHeader) {};

    virtual void print(std::ostream& stream) const;

    std::int32_t numFrames = 0;
    // 80-bit extended value.
    // Array ordered in big-endian, but each cell is in native endianness.
    // AIFFSampleRate is essentially the same value as the standard header
    // sampleRate.
    std::uint32_t AIFFSampleRate[3] = {0};
    std::uint32_t markerChunk = 0;
    std::uint8_t format[4] = {0}; // 4-char string.
    std::int32_t futureUse2 = 0;
    std::uint32_t stateVars = 0;
    std::uint32_t leftOverSamples = 0;

    std::int16_t compressionID = 0;
    std::int16_t packetSize = 0;
    std::int16_t snthID = 0;
    std::int16_t sampleSize = 0;
};

std::ostream& operator<<(std::ostream& lhs, const SoundSampleHeader& rhs);

#endif // SOUND_SAMPLE_HEADER_HPP
