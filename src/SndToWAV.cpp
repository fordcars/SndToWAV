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

#include "SndToWAV.hpp"

#include "SndFile.hpp"
#include "WAVFile.hpp"

#ifdef USE_UNIVERSAL_ENDIANNESS_HACK
    // Is not guaranteed to work on all compilers.
    // From David Cournapeau
    // (https://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program)
    static bool isMachineLittleEndianHack()
    {
        union {
            uint32_t i;
            char c[4];
        } bint = {0x01020304};

        return !(bint.c[0] == 1);
    }

    const bool SndToWAV::mMachineIsLittleEndian = isMachineLittleEndianHack();
#elif defined(ON_LITTLE_ENDIAN_MACHINE)
    const bool SndToWAV::mMachineIsLittleEndian = true;
#else
    // Big-endian machine assumed, don't change endianness!
    const bool SndToWAV::mMachineIsLittleEndian = false;
#endif /* USE_UNIVERSAL_ENDIANNESS_HACK*/

SndToWAV::SndToWAV()
{
    
}

// Returns true on success, false on failure.
bool SndToWAV::convert(const std::string& sndFileName, const std::string& wavFileName)
{
    SndFile sndFile(sndFileName);
	WAVFile wavFile;
    bool success = wavFile.convertSnd(sndFile, wavFileName);

    if(success)
    {
        std::cout <<
            std::endl << "Successfully converted '" + sndFileName + "' to '" +
            wavFileName + "'!" << std::endl;
        return true;
    }

    std::cerr <<
        std::endl << "Error: failed to convert '" + sndFileName + "' to '" +
        wavFileName + "'!" << std::endl;
    return false;
}
