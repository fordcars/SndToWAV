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

#ifndef SND_TO_WAV_HPP
#define SND_TO_WAV_HPP

#include <string>
#include <cstddef> // For size_t
#include <climits> // For CHAR_BIT
#include <cstddef> // For size_t

// Define if you are compiling for a little-endian machine.
// When undefined, big-endian machine is assumed (unless universal
// endianness hack is used).
// #define ON_LITTLE_ENDIAN_MACHINE

// If defined, will use union hack to figure out the machine's endianness
// at run-time. Use this if you wish to detect endianness automatically.
// Might cause undefined behavior/compile-time error on future compilers.
// Setting this to true will ignore ON_LITTLE_ENDIAN_MACHINE.
#define USE_UNIVERSAL_ENDIANNESS_HACK

class SndToWAV
{
private:
    // From https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
    template<typename T>
    static T swapEndian(T u)
    {
        static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

        union
        {
            T u;
            unsigned char u8[sizeof(T)];
        } source, dest;

        source.u = u;

        for (size_t k = 0; k < sizeof(T); k++)
            dest.u8[k] = source.u8[sizeof(T) - k - 1];

        return dest.u;
    }

public:
    // u is big-endian value.
    template<class T>
    static T makeSafeEndian(T u)
    {
        if(mMachineIsLittleEndian)
            return swapEndian(u);
        else
            return u;
    }

    SndToWAV();

    static const bool mMachineIsLittleEndian;

    bool convert(const std::string& sndFileName, const std::string& wavFileName);
    
};

#endif // SND_TO_WAV_HPP
