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

#ifndef WAV_FILE_HPP
#define WAV_FILE_HPP

#include <ostream>
#include <string>
#include <cstdint> // Fixed-width types
#include <vector>

#include "SndToWAV.hpp"

class SndFile;
class WAVHeader
{
public:
    // http://soundfile.sapp.org/doc/WaveFormat/
    std::uint8_t chunkID[4] = {'R','I','F','F'};
    std::uint32_t chunkSize = 0;
    std::uint8_t format[4] = {'W','A','V','E'};

    std::uint8_t subchunk1ID[4] =  {'f','m','t',' '};
    std::uint32_t subchunk1Size = 0;
    std::uint16_t audioFormat = 0;
    std::uint16_t numChannels = 0;
    std::uint32_t sampleRate = 0;
    std::uint32_t byteRate = 0;
    std::uint16_t blockAlign = 0;
    std::uint16_t bitsPerSample = 0;

    std::uint8_t subchunk2ID[4] = {'d','a','t','a'};
    std::uint32_t subchunk2Size = 0;

    friend std::ostream& operator<<(std::ostream& lhs, const WAVHeader& rhs);
};

class WAVFile
{
private:
    WAVHeader mHeader;

    // Safe endian.
    // littleStream is a little-endian output stream.
    template<class T>
    static void writeLittleValue(std::ostream& littleStream, T value)
    {
        T little = SndToWAV::safeLittleEndian(value);
        littleStream.write(reinterpret_cast<const char*>(&little), sizeof(T));
    }

    // Safe endian.
    // littleStream is a little-endian output stream.
    // Only writes length of bytes (LSBs of value).
    template<class T>
    static T writeLittleValue(std::ostream& littleStream, T value, std::size_t length)
    {
        T little = SndToWAV::safeLittleEndian(value);
        
        littleStream.write(reinterpret_cast<const char*>(&little), length);
    }

    // Safe endian.
    // littleStream is a little-endian output stream.
    template<class T>
    static void writeLittleArray(std::ostream& littleStream, T* values, std::size_t length)
    {
        for(std::size_t i = 0; i < length; ++i)
        {
            T little = SndToWAV::safeLittleEndian(values[i]);
            littleStream.write(reinterpret_cast<const char*>(&little), sizeof(T));
        }
    }

    bool populateHeader(const SndFile& sndFile);
    void writeBinaryHeader(std::ostream& outputStream) const;
    std::vector<std::uint8_t> decodeSampleData(const SndFile& sndFile) const;
    bool writeSampleData(std::ostream& outputStream, const SndFile& sndFile) const;

public:
    WAVFile();
    WAVFile(const SndFile& sndFile, const std::string& WAVFileName);

    bool convertSnd(const SndFile& sndFile, const std::string& WAVFileName);
};

#endif // WAV_FILE_HPP
