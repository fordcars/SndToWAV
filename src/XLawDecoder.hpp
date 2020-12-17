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

#ifndef XLAW_DECODER_HPP
#define XLAW_DECODER_HPP

#include "Decoder.hpp"

#include <cstddef>
#include <vector>
#include <cstdint>

class XLawDecoder : public Decoder
{
protected:
    XLawDecoder();

    std::size_t getCompressedSize(std::size_t numFrames,
        std::size_t numChannels) override;

    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels, bool useULaw);
};

class ALawDecoder : public XLawDecoder
{
public:
    ALawDecoder();
    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) override;
};


class ULawDecoder : public XLawDecoder
{
public:
    ULawDecoder();
    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) override;
};

#endif // XLAW_DECODER_HPP
