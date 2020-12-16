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

#include "Log.hpp"
#include <iostream>

std::stringstream Log::mDeadStream;

std::ostream Log::info(std::cout.rdbuf());
std::ostream Log::warn(std::cout.rdbuf());
std::ostream Log::err(std::cerr.rdbuf());

// Initilally, verbose is disabled; it will not print anything.
std::ostream Log::verb(Log::mDeadStream.rdbuf());

// Static
void Log::setVerbose(bool verboseOn)
{
    if(verboseOn)
    {
        verb.rdbuf(std::cout.rdbuf());
    } else
    {
        verb.rdbuf(mDeadStream.rdbuf());
    }
}

