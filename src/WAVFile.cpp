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

#include "WAVFile.hpp"
#include "SndFile.hpp"

#include <iomanip>
#include <cstddef> // For std::size_t

std::ostream& operator<<(std::ostream& lhs, const WAVHeader& rhs)
{
    lhs << std::setfill('0'); // For hex values
    lhs <<
        "Generated WAV file header:" << std::endl <<
        " -- Chunk ID: " << std::string(reinterpret_cast<const char*>(rhs.chunkID), 4) <<
            std::endl <<
        " -- Chunk size: " << rhs.chunkSize << std::endl <<
        " -- Format: " << std::string(reinterpret_cast<const char*>(rhs.format), 4) <<
            std::endl <<

        " -- Subchunk 1 ID: " << std::string(reinterpret_cast<const char*>(rhs.subchunk1ID), 4) <<
            std::endl <<
        " -- Subchunk 1 size: " << rhs.subchunk1Size << std::endl <<
        " -- Audio format: " << "0x" << std::setw(4) << std::hex <<
            rhs.audioFormat  << std::endl <<
        " -- Number of channels: " << std::dec << rhs.numChannels << std::endl <<
        " -- Sample rate: " << rhs.sampleRate << std::endl <<
        " -- Byte rate: " << rhs.byteRate << std::endl <<
        " -- Block align: " << rhs.blockAlign << std::endl <<
        " -- Bits per sample: " << rhs.bitsPerSample << std::endl <<

        " -- Subchunk 2 ID: " << std::string(reinterpret_cast<const char*>(rhs.subchunk2ID), 4) <<
            std::endl <<
        " -- Subchunk 2 size: " << rhs.subchunk2Size;
        

    lhs << std::dec << std::setfill(' '); // Restore
    return lhs;
}

WAVFile::WAVFile()
{
}

WAVFile::WAVFile(const SndFile& sndFile, const std::string& WAVFileName)
{
    convertSnd(sndFile, WAVFileName);
}

// Returns true on success, false on failure.
bool WAVFile::populateHeader(const SndFile& sndFile)
{
    // Could definitely do this cleaner.
    const SoundSampleHeader& sndHeader = sndFile.getSoundSampleHeader();
    const ExtendedSoundSampleHeader* extSndHeader = nullptr;
    const CompressedSoundSampleHeader* cmpSndHeader = nullptr;

    if(sndHeader.encode == SndFile::cExtendedSoundHeaderEncode)
    {
        extSndHeader = static_cast<const ExtendedSoundSampleHeader*>(&sndHeader);
    } else if(sndHeader.encode == SndFile::cCompressedSoundHeaderEncode)
    {
        cmpSndHeader = static_cast<const CompressedSoundSampleHeader*>(&sndHeader);
    }

    std::size_t sampleDataSize = 0;
    if(sndHeader.encode == SndFile::cStandardSoundHeaderEncode)
    {
        sampleDataSize = sndHeader.lengthOrChannels;
    } else if(sndHeader.encode == SndFile::cExtendedSoundHeaderEncode)
    {
        sampleDataSize = extSndHeader->numFrames * 2 * sndHeader.lengthOrChannels;
    } else if(sndHeader.encode == SndFile::cCompressedSoundHeaderEncode)
    {
        sampleDataSize = cmpSndHeader->numFrames * 128 * sndHeader.lengthOrChannels;
    }

    mHeader.chunkSize = 36 + sampleDataSize;

    // "fmt " //
    mHeader.subchunk1Size = 16;
    mHeader.audioFormat = 1; // For PCM

    if(sndHeader.encode == SndFile::cStandardSoundHeaderEncode)
    {
        mHeader.numChannels = 1; // Only mono supported.
    } else if(sndHeader.encode == SndFile::cExtendedSoundHeaderEncode ||
        sndHeader.encode == SndFile::cCompressedSoundHeaderEncode )
    {
        mHeader.numChannels = sndHeader.lengthOrChannels;
    }

    // Snd sample rate is an unsigned 32-bit fixed-point.
    // We only keep the integer part!
    mHeader.sampleRate = sndHeader.sampleRate >> 16;

    unsigned bitsPerSample = 0;

    if(sndHeader.encode == SndFile::cStandardSoundHeaderEncode)
    {
        // For standard sound headers, samples are always 8-bit.
        bitsPerSample = 8;
    } else if(sndHeader.encode == SndFile::cExtendedSoundHeaderEncode)
    {
        bitsPerSample = extSndHeader->sampleSize;
    } else if(sndHeader.encode == SndFile::cCompressedSoundHeaderEncode)
    {
        // If == 0, we are supposed to guess the packet size :(
        if(cmpSndHeader->packetSize == 0)
            // Normally 16-bit, but not always! This may fail.
            bitsPerSample = 16;
        else
            bitsPerSample = cmpSndHeader->packetSize;
    }

    mHeader.byteRate = mHeader.sampleRate * mHeader.numChannels * bitsPerSample/8;
    mHeader.blockAlign = mHeader.numChannels * bitsPerSample/8;
    mHeader.bitsPerSample = bitsPerSample;

    // "data" //
    mHeader.subchunk2Size = sampleDataSize;

    // Debug info.
    std::cout << mHeader << std::endl;

    return true;
}

void WAVFile::writeBinaryHeader(std::ostream& outputStream) const
{
    writeLittleArray(outputStream, mHeader.chunkID, 4);
    writeLittleValue(outputStream, mHeader.chunkSize);
    writeLittleArray(outputStream, mHeader.format, 4);

    writeLittleArray(outputStream, mHeader.subchunk1ID, 4);
    writeLittleValue(outputStream, mHeader.subchunk1Size);
    writeLittleValue(outputStream, mHeader.audioFormat);
    writeLittleValue(outputStream, mHeader.numChannels);
    writeLittleValue(outputStream, mHeader.sampleRate);
    writeLittleValue(outputStream, mHeader.byteRate);
    writeLittleValue(outputStream, mHeader.blockAlign);
    writeLittleValue(outputStream, mHeader.bitsPerSample);

    writeLittleArray(outputStream, mHeader.subchunk2ID, 4);
    writeLittleValue(outputStream, mHeader.subchunk2Size);
}

void WAVFile::writeSampleData(std::ostream& outputStream, const SndFile& sndFile) const
{
    const SoundSampleHeader& sndHeader = sndFile.getSoundSampleHeader();

    if(sndHeader.encode == SndFile::cStandardSoundHeaderEncode ||
        sndHeader.encode == SndFile::cExtendedSoundHeaderEncode)
    {
        // Snd only supports 8-bit or 16-bit samples (I think).
        unsigned bytesPerSample = mHeader.bitsPerSample/8;
        if(bytesPerSample == 1)
        {
            writeLittleArray(outputStream, sndHeader.samples.data(),
                sndHeader.samples.size());
        } else if(bytesPerSample == 2)
        {
            // If we have 2 bytes/sample, we have to make sure each sample is in little-endian!
            writeLittleArray(outputStream,
                reinterpret_cast<const std::uint16_t*>(sndHeader.samples.data()),
                sndHeader.samples.size() / bytesPerSample);
        }
    }
}

// Returns true on success, false on failure.
bool WAVFile::convertSnd(const SndFile& sndFile, const std::string& WAVFileName)
{
    if(!populateHeader(sndFile))
        return false; // Error messages already dealt with.

    std::ofstream outputFile(WAVFileName, std::ofstream::out |
            std::ofstream::binary | std::ofstream::trunc);

    if(outputFile.fail())
    {
        std::cerr << "Error: could not open '" + WAVFileName + "' for writing!" <<
            std::endl;
        return false;
    }

    writeBinaryHeader(outputFile);
    writeSampleData(outputFile, sndFile);
}
