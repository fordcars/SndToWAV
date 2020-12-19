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

#include "SoundSampleHeader.hpp"
#include <iomanip>

namespace
{
    // sampleRate is an unsigned 32-bit fixed-point sample rate:
    // the 16 MSbs are left of the point, and the 16 LSbs are right of the point.
    float fixedSampleRateToFloat(std::uint32_t sampleRate)
    {
        return sampleRate / (16.0f*16.0f*16.0f*16.0f);
    }
}

void SoundSampleHeader::print(std::ostream& stream) const
{
    stream << std::setfill('0'); // For hex values

    stream <<
        "Base sound sample header: " << std::endl <<
        " -- Sample pointer: " << "0x" << std::setw(8) <<
            std::hex << samplePtr << std::endl <<

        " -- Length or num. channels: " << std::dec << lengthOrChannels << std::endl <<
        " -- Total sample area size: " << sampleArea.size() << std::endl <<
        " -- Sample rate: " << "0x" << std::setw(8) << std::hex <<
            sampleRate << std::dec << " (" << fixedSampleRateToFloat(sampleRate) <<
            " Hz)" << std::endl <<

        " -- Loop start: " << "0x" << std::setw(8) << std::hex <<
            loopStart  << std::endl <<
        " -- Loop end: " << "0x" << std::setw(8) << loopEnd << std::endl <<
        " -- Encoding: " << "0x" << std::setw(2) <<
            static_cast<unsigned>(encode) << std::endl <<

        " -- Base frequency: " << "0x" << std::setw(2) << 
            static_cast<unsigned>(baseFrequency);

    stream << std::dec << std::setfill(' '); // Restore stream
}

void ExtendedSoundSampleHeader::print(std::ostream& stream) const
{
    SoundSampleHeader::print(stream); // Print base class

    stream << std::setfill('0'); // For hex values
    stream << std::endl <<
        "Extended sound sample header (0xff): " << std::endl <<
        " -- Number of frames: " << numFrames << std::endl <<

        // Array in big-endian!
        " -- AIFFSampleRate: " << "0x" << std::hex << std::setw(4) << AIFFSampleRate[0] <<
            std::setw(8) << AIFFSampleRate[1] << std::setw(8) << AIFFSampleRate[2] <<
            std::endl <<

        " -- Marker chunk pointer: " << "0x" << std::setw(8) << markerChunk << std::endl <<
        " -- Instrument chunks pointer: " << "0x" << std::setw(8) <<
            instrumentChunks << std::endl <<
        " -- AES Recording pointer: " << "0x" << std::setw(8) << AESRecording << std::endl <<
        " -- Sample size: " << std::dec << sampleSize << std::endl <<
        " -- Future use (1): " << "0x" << std::hex << std::setw(4) << futureUse1 << std::endl <<
        " -- Future use (2): " << "0x" << std::setw(8) << futureUse2 << std::endl <<
        " -- Future use (3): " << "0x" << std::setw(8) << futureUse3 << std::endl <<
        " -- Future use (4): " << "0x" << std::setw(8) << futureUse4;

    stream << std::dec << std::setfill(' '); // Restore stream
}

void CompressedSoundSampleHeader::print(std::ostream& stream) const
{
    SoundSampleHeader::print(stream); // Print base class

    stream << std::setfill('0'); // For hex values
    stream << std::endl <<
        "Compressed sound sample header (0xfe): " << std::endl <<
        " -- Number of frames: " << numFrames << std::endl <<

        // Array in big-endian
        " -- AIFFSampleRate: " << "0x" << std::hex << std::setw(4) <<AIFFSampleRate[0] <<
            std::setw(8) << AIFFSampleRate[1] << std::setw(8) << AIFFSampleRate[2] <<
            std::endl <<

        " -- Marker chunk pointer: " << "0x" << std::setw(8) << markerChunk << std::endl <<
        " -- Format: " << std::string(reinterpret_cast<const char*>(format), 4) << std::endl <<
        " -- Future use (2): " << "0x" << std::setw(8) << futureUse2 << std::endl <<
        " -- State vars pointer: " << "0x" << std::setw(8) << stateVars << std::endl <<
        " -- Leftover samples pointer: " << "0x" << std::setw(8) << leftOverSamples << std::endl <<
        " -- Compression ID: " << std::dec << compressionID << std::endl <<

        " -- Packet size: " << packetSize << std::endl <<
        " -- Snth ID: " << "0x" << std::setw(4) <<
            std::hex << snthID << std::endl <<
        " -- Sample size: " << std::dec << sampleSize;

    stream << std::dec << std::setfill(' '); // Restore stream
}

std::ostream& operator<<(std::ostream& lhs, const SoundSampleHeader& rhs)
{
    rhs.print(lhs);
    return lhs;
}
