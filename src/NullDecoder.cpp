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

// Use this pass-through class for uncompressed sound.

#include "NullDecoder.hpp"

NullDecoder::NullDecoder(unsigned bitsPerSample)
    : mBitsPerSample(bitsPerSample)
{

}

// For uncompressed sound, numPackets = number of samples.
std::size_t NullDecoder::getEncodedSize(std::size_t numPackets) const
{
    // Always in bytes.
    return numPackets * mBitsPerSample/8;
}

// For uncompressed sound, numPackets = number of samples.
std::size_t NullDecoder::getDecodedSize(std::size_t numPackets) const
{
    // Always in bytes.
    return numPackets * mBitsPerSample/8;
}

unsigned NullDecoder::getBitsPerSample() const
{
    return mBitsPerSample;
}

void NullDecoder::decode(const std::vector<std::uint8_t>& data,
    std::size_t /* numChannels */)
{
    setLittleEndianData(data);
}

