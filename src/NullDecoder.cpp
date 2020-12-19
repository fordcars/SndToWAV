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
#include "Log.hpp"

NullDecoder::NullDecoder(unsigned bitsPerSample)
    : mBitsPerSample(bitsPerSample)
{

}

// Big-endian std::vector<std::uint8_t> to native-endian
// std::vector<std::int16_t>.
std::vector<std::int16_t> NullDecoder::bigDataTo16BitSamples(
    const std::vector<std::uint8_t>& data)
{
    std::vector<std::int16_t> samples(data.size()/2);

    if(data.size()%2 != 0)
    {
        Log::err << "Error: 16-bit samples do not contain an even number " <<
            "of bytes!" << std::endl;
        return samples;
    }

    for(std::size_t i = 0; i < data.size()/2; ++i)
    {
        // Because of << and &, this is platform-independant.
        std::uint16_t unsignedValue =
            (static_cast<std::uint16_t>(data[i*2]) << 8) | data[i*2+1];

        samples[i] = *reinterpret_cast<std::int16_t*>(&unsignedValue);
    }

    return samples;
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

// data is Big-endian!
bool NullDecoder::decode(const std::vector<std::uint8_t>& data,
    std::size_t /* numChannels */)
{
    if(mBitsPerSample == 8)
    {
        setLittleEndianData(data);
        return true;
    } else if(mBitsPerSample == 16)
    {
        // Convert the Big-endian sample data to little-endian samples.
        setLittleEndianData(
            bigDataTo16BitSamples(data)
        );
        return true;
    }

    Log::err << "Error: " << mBitsPerSample << "-bit samples not supported " <<
        "for uncompressed sound!" << std::endl;
    return false;
}

