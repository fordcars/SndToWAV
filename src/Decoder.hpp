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
private:
    std::vector<std::uint8_t> mLittleEndianData;

    static std::vector<uint8_t> serializeToLittleEndian(const std::vector<std::int16_t>& data);

protected:
    void setLittleEndianData(const std::vector<std::int16_t>& samples);
    void setLittleEndianData(const std::vector<std::int8_t>& samples);
    void setLittleEndianData(const std::vector<std::uint8_t>& data);

public:
    virtual ~Decoder() = default;

    // Returns size of compressed samples, in bytes.
    virtual std::size_t getEncodedSize(std::size_t numPackets) const = 0;

    // Returns size of uncompressed samples, in bytes.
    virtual std::size_t getDecodedSize(std::size_t numPackets) const = 0;

    // Bits per uncompressed sample.
    virtual unsigned getBitsPerSample() const = 0;

    virtual void decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) = 0;

    const std::vector<std::uint8_t> getLittleEndianData() const;
};

#endif // DECODER_HPP
