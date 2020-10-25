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

#include <iostream>
#include <string>
#include <cstdint> // Fixed-width types

class SndFile;
class WAVHeader
{
public:
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

    bool populateHeader(const SndFile& sndFile);

public:
    WAVFile();
    WAVFile(const SndFile& sndFile, const std::string& WAVFileName);

    bool convertSnd(const SndFile& sndFile, const std::string& WAVFileName);
};

#endif // WAV_FILE_HPP
