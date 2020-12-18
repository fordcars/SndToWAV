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

#ifndef IMA4_DECODER_HPP
#define IMA4_DECODER_HPP

#include "Decoder.hpp"

#include <cstdint>
#include <cstddef>
#include <vector>

const unsigned IMA4_PACKET_LENGTH = 34;

class IMA4Decoder : public Decoder
{
private:
    template<class T>
    static T clamp(long long int min, T value, long long int max)
    {
        if(value < min) return min;
        if(value > max) return max;
        return value;
    }

    // Will cast 16-bit value as a signed value, assuming two's compliment.
    // Why write this? For platform-independance.
    template<class TDest>
    static TDest castSigned16Bit(std::uint16_t value)
    {
        // Two's complement:
        std::int16_t positivePart = value & 0x7fff; // 15 LSbits

        // If MSb is 1, we are dealing with a negative value:
        bool isNegative = (value & 0x8000) == 0x8000;
       
        if(isNegative)
            return static_cast<TDest>(positivePart) - 0x8000; // Subtract sign-bit value
        else
            return static_cast<TDest>(positivePart);
    }

    std::int16_t processNibble(std::uint8_t nibble);
    std::vector<std::int16_t> decodeFrame(const std::uint8_t frame[IMA4_PACKET_LENGTH]);
    std::vector<std::int16_t> decodeStereoFrame(const std::uint8_t leftFrame[IMA4_PACKET_LENGTH],
        const std::uint8_t rightFrame[IMA4_PACKET_LENGTH]);

    // Step index must be a signed value, even if it is clamped!
    // Failing to do so will result in problematic sound due to overflow.
    int mStepIndex = 0;
    int mPredictor = 0;

public:
    IMA4Decoder();

    std::size_t getEncodedSize(std::size_t numPackets) const override;
    std::size_t getDecodedSize(std::size_t numPackets) const override;

    unsigned getBitsPerSample() const override;

    void decode(const std::vector<std::uint8_t>& data,
        std::size_t numChannels) override;
};

#endif // IMA4_DECODER_HPP
