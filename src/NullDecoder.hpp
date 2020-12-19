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

#ifndef NULL_DECODER_HPP
#define NULL_DECODER_HPP

#include "Decoder.hpp"

#include <cstdint>
#include <cstddef>
#include <vector>

class NullDecoder : public Decoder
{
private:
    static std::vector<std::int16_t> bigDataTo16BitSamples(
        const std::vector<std::uint8_t>& data);

    unsigned mBitsPerSample;

public:
    NullDecoder(unsigned bitsPerSample);

    std::size_t getEncodedSize(std::size_t numPackets) const override;
    std::size_t getDecodedSize(std::size_t numPackets) const override;

    unsigned getBitsPerSample() const override;

    bool decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) override;
};

#endif // NULL_DECODER_HPP
