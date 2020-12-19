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

#include "Decoder.hpp"

// Static
// Convert std::vector<std::int16_t> native-endian values to
// std::vector<std::uint8_t> in little-endian.
std::vector<uint8_t> Decoder::serializeToLittleEndian(const std::vector<std::int16_t>& data)
{
    std::vector<std::uint8_t> convertedData(data.size() * 16/8);

    for(std::size_t i = 0; i < data.size(); ++i)
    {
        // Signed to unsigned:
        std::uint16_t unsignedValue =
            *reinterpret_cast<const std::uint16_t*>( &(data[i]) );

        // LSB goes to smallest adress.
        // '&' makes this platform-independant.
        convertedData[i*2] = static_cast<std::uint8_t>(unsignedValue & 0x00FF);
        
        // MSB goes to largest adress.
        // '>>' makes this platform-independant.
        convertedData[i*2 + 1] = static_cast<std::uint8_t>(unsignedValue >> 8);
    }

    return convertedData;
}

void Decoder::setLittleEndianData(const std::vector<std::int16_t>& samples)
{
    mLittleEndianData = serializeToLittleEndian(samples);
}

void Decoder::setLittleEndianData(const std::vector<std::int8_t>& samples)
{
    // Conversion exists between int8_t and uint8_t.
    mLittleEndianData = std::vector<std::uint8_t>(samples.begin(), samples.end());
}

// For raw data.
void Decoder::setLittleEndianData(const std::vector<std::uint8_t>& data)
{
    mLittleEndianData = data;
}

const std::vector<std::uint8_t> Decoder::getLittleEndianData() const
{
    return mLittleEndianData;
}
