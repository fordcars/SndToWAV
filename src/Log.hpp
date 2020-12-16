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

#ifndef LOG_HPP
#define LOG_HPP

#include <ostream>
#include <sstream>

class Log
{
private:
    static std::stringstream mDeadStream; // Will not print anything.

public:
    static std::ostream info; // Normal logging
    static std::ostream warn; // Warning
    static std::ostream err;  // Error
    static std::ostream verb; // Verbose

    static void setVerbose(bool verboseOn);
};

#endif // LOG_HPP
