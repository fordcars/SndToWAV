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

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef> // For size_t
#include <climits> // For CHAR_BIT

// Define if you are compiling for a little-endian machine.
// When undefined, big-endian machine is assumed.
#define ON_LITTLE_ENDIAN_MACHINE

namespace Utils
{
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

        for (std::size_t k = 0; k < sizeof(T); k++)
            dest.u8[k] = source.u8[sizeof(T) - k - 1];

        return dest.u;
    }

    // Converts Big-endian to native-endian.
    template<class T>
    static T makeBigEndianNative(T big)
    {
#ifdef ON_LITTLE_ENDIAN_MACHINE
            return swapEndian(big);
#else
            return big;
#endif
    }

    // Converts little-endian to native-endian.
    template<class T>
    static T makeLittleEndianNative(T little)
    {
#ifdef ON_LITTLE_ENDIAN_MACHINE
            return little;
#else
            return swapEndian(little);
#endif
    }

    // Converts native-endian to Big-endian.
    template<class T>
    static T safeBigEndian(T nativeEndian)
    {
#ifdef ON_LITTLE_ENDIAN_MACHINE
            return swapEndian(nativeEndian);
#else
            return nativeEndian;
#endif
    }

    // Converts native-endian to little-endian.
    template<class T>
    static T safeLittleEndian(T nativeEndian)
    {
#ifdef ON_LITTLE_ENDIAN_MACHINE
            return nativeEndian;
#else
            return swapEndian(nativeEndian);
#endif
    }
}

#endif // UTILS_HPP
