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

#ifndef DECODER_HPP
#define DECODER_HPP

#include <cstdint>
#include <cstddef>
#include <vector>

class Decoder
{
public:
    virtual ~Decoder() = default;

    // Returns size of compressed samples, in bytes.
    virtual std::size_t getEncodedSize(std::size_t numPackets) = 0;

    // Returns size of uncompressed samples, in bytes.
    virtual std::size_t getDecodedSize(std::size_t numPackets) = 0;

    // Bits per uncompressed sample.
    virtual unsigned getBitsPerSample() = 0;

    virtual std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) = 0;
};

#endif // DECODER_HPP
