# Copyright 2020 Carl Hewett
#
# This file is part of SndToWAV.
#
# SndToWAV is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SndToWAV is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SndToWAV. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.7.0)
project(SndToWAVProject)

# Definitions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQURED ON)

# CMAKE_SOURCE_DIR: path to directory of the top-level 'CMakeLists.txt'.
# CMAKE_BINARY_DIR: path to where we ran 'cmake' from.

set(SNDTOWAV_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(SNDTOWAV_OUTPUT_EXE_DIR ${CMAKE_BINARY_DIR}/../bin)

set(SNDTOWAV_SOURCES
	${SNDTOWAV_SOURCE_DIR}/main.cpp
    ${SNDTOWAV_SOURCE_DIR}/SndToWAV.cpp
    ${SNDTOWAV_SOURCE_DIR}/SndFile.cpp
	${SNDTOWAV_SOURCE_DIR}/WAVFile.cpp
)

set(SNDTOWAV_HEADERS
	${SNDTOWAV_SOURCE_DIR}/main.hpp
    ${SNDTOWAV_SOURCE_DIR}/SndToWAV.hpp
    ${SNDTOWAV_SOURCE_DIR}/SndFile.hpp
	${SNDTOWAV_SOURCE_DIR}/WAVFile.hpp
)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

# Output executable to output directory
# Must be set BEFORE calling add_executable()!!
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SNDTOWAV_OUTPUT_EXE_DIR})

# Create cmdline executable
add_executable(
	SndToWAV
    
    ${SNDTOWAV_SOURCES}
    ${SNDTOWAV_HEADERS}
)

# Add executable includes
message(STATUS "Adding executable includes...")
target_include_directories(
	SndToWAV
	PRIVATE ${SNDTOWAV_SOURCE_DIR}
)

# Fix Microsoft.
# Unlike other compilers where we can check
# the CMAKE_CXX_COMPILER_ID string, Microsoft Visual C++
# has its own constant. MSVC is set to true if we are using
# a version of Microsoft Visual C++.
if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG
		"${CMAKE_CXX_FLAGS_DEBUG} /wd4100 /wd4127 /wd4189 /wd4512 /wd4702")
		
	# Repairs conflicts with Windows libraries, or tries to.
	# If this doesn't work, building as release will work.
	set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"LIBCMT, MSVCRT, MSVPRTD\")

	set(CMAKE_FIND_LIBRARY_PREFIXES "")
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
endif()

# Fix GNU
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	# Repeated calls for the same target append the items
	# in the order called.
	target_link_libraries(
		SndToWAV

		# To remove missing DSO symbol error, it has
		# to be here (annoyingly).
		# This is actually a library called libdl, which
		# is the Dynamic Linking library.
		PRIVATE -ldl
	)
endif()