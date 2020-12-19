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

#ifndef SND_TO_WAV_HPP
#define SND_TO_WAV_HPP

#include "ResExtractor.hpp"

#include <string>
#include <cstddef> // For size_t
#include <memory>

class SndToWAV
{
private:
    static void printResult(bool success, const std::string& name,
        const std::string& wavFileName);

    bool convertResourceData(char* resourceData, std::size_t resourceSize,
        const std::string& name);

    std::size_t mResourceFileBlockSize;

public:
    SndToWAV(std::size_t resourceFileBlockSize);

    bool extract(const std::string& resourceFilePath, unsigned int resourceID);
    bool extract(const std::string& resourceFilePath, const std::string& resourceName);
    bool extract(const std::string& resourceFilePath);

    static const bool mMachineIsLittleEndian;
};

#endif // SND_TO_WAV_HPP
