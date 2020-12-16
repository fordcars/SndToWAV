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

#include <vector>
#include <cstdint>

class MACEDecoder
{
public:
    MACEDecoder();

    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels);
};

#endif // MACE_DECODER_HPP
