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

#ifndef MACE_DECODER_HPP
#define MACE_DECODER_HPP

#include "Decoder.hpp"

#include <vector>
#include <cstddef>
#include <cstdint>

class MACEDecoder : public Decoder
{
public:
    MACEDecoder();

    std::size_t getEncodedSize(std::size_t numPackets) const override;
    std::size_t getDecodedSize(std::size_t numPackets) const override;

    unsigned getBitsPerSample() const override;

    void decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) override;
};

#endif // MACE_DECODER_HPP
