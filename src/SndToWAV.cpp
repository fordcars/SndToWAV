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
#include "Log.hpp"
#include "SndFile.hpp"
#include "WAVFile.hpp"

#include <sstream>

// Block size is often 4096 bytes.
SndToWAV::SndToWAV(std::size_t resourceFileBlockSize)
    : mResourceFileBlockSize(resourceFileBlockSize)
{

}

// Static
void SndToWAV::printResult(bool success, const std::string& name,
    const std::string& wavFileName)
{
    if(success)
    {
        Log::info << "Extracted '" + name + "' to '" +
            wavFileName + "'!" << std::endl;
    } else
    {
        Log::err <<  "Error: failed to convert '" + name + "' to '" +
            wavFileName + "'!" << std::endl;
    }
}

// Converts char* containing an 'snd ' resource to wav.
// Returns true on success, false on failure
bool SndToWAV::convertResourceData(char* resourceData, std::size_t resourceSize,
    const std::string& name)
{
    // Convert char* to stream.
    std::stringstream stream;
    stream.rdbuf()->pubsetbuf(resourceData, resourceSize);

    SndFile sndFile(stream, name);
	WAVFile wavFile;
    std::string wavFileName = name + ".wav";
    bool success = wavFile.convertSnd(sndFile, wavFileName);
    printResult(success, name, wavFileName);

    return success;
}

// Convert an 'snd ' resource by ID.
// Returns true on success, false on failure
bool SndToWAV::extract(const std::string& resourceFilePath, unsigned int resourceID)
{
    RESX::File resourceFile(resourceFilePath, mResourceFileBlockSize);
    std::size_t resourceSize = 0;
    std::unique_ptr<char, RESX::freeDelete> resourceData =
        resourceFile.loadResourceFork(0).getResourceData("snd ", resourceID, &resourceSize);

    if(resourceData == nullptr)
    {
        Log::err << "Error: could not find sound with ID '" << resourceID << "' in '" <<
            resourceFilePath << "'!" << std::endl;
        return false;
    }
    
    return convertResourceData(resourceData.get(), resourceSize, std::to_string(resourceID));
}

// Convert an 'snd ' resource by name.
// Returns true on success, false on failure
bool SndToWAV::extract(const std::string& resourceFilePath, const std::string& resourceName)
{
    RESX::File resourceFile(resourceFilePath, mResourceFileBlockSize);
    std::size_t resourceSize;
    std::unique_ptr<char, RESX::freeDelete> resourceData =
        resourceFile.loadResourceFork(0).getResourceData("snd ", resourceName, &resourceSize);

    if(resourceData == nullptr)
    {
        Log::err << "Error: could not find sound '" << resourceName << "' in '" <<
            resourceFilePath << "'!" << std::endl;
        return false;
    }

    return convertResourceData(resourceData.get(), resourceSize, resourceName);
}

// Convert all 'snd ' resources in resource file.
// Returns true on success, false on failure
bool SndToWAV::extract(const std::string& resourceFilePath)
{
    RESX::File resourceFile(resourceFilePath, mResourceFileBlockSize);

    // Get all "snd " resources' names.
    RESX::ResourceFork resourceFork = resourceFile.loadResourceFork(0);
    std::vector<std::string> names = resourceFork.getResourcesNames("snd ");

    bool success = true;

    for(const std::string& name : names)
    {
        success &= extract(resourceFilePath, name);
        Log::verb << std::endl; // To avoid cluttered verbose output.
    }

    return success;
}

