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

#ifndef SND_FILE_HPP
#define SND_FILE_HPP

#include "Utils.hpp"
#include "SoundSampleHeader.hpp"
#include "Decoder.hpp"

#include <string>
#include <istream>
#include <ostream>
#include <cstdint> // Fixed-width types
#include <cstddef> // For std::size_t
#include <vector>
#include <memory>

class SndFile
{
private:
    // Will return native-endian value.
    // bigStream is a Big-endian input stream.
    template<class T>
    static T readBigValue(std::istream& bigStream)
    {
        T readBigValue;
        bigStream.read(reinterpret_cast<char*>(&readBigValue), sizeof(T));
        return Utils::makeBigEndianNative(readBigValue);
    }

    // Will return native-endian value.
    // bigStream is a Big-endian input stream.
    // Only reads length bytes as LSBs, fills rest (MSBs) with 0s.
    template<class T>
    static T readBigValue(std::istream& bigStream, std::size_t length)
    {
        T readBigValue = 0; // Will with 0s.
        // Fill data in the LSBs of readValue (as Big-endian).
        bigStream.read(reinterpret_cast<char*>(&readBigValue) + (sizeof(T)-length), length);
        return Utils::makeBigEndianNative(readBigValue);
    }

    // Will return native-endian values.
    // bigStream is a Big-endian input stream.
    template<class T>
    static void readBigArray(std::istream& bigStream, T* buffer, std::size_t length)
    {
        for(std::size_t i = 0; i < length; ++i)
        {
            bigStream.read(reinterpret_cast<char*>(&buffer[i]), sizeof(T));
            buffer[i] = Utils::makeBigEndianNative(buffer[i]);
        }
    }

    std::string mFileName;
    std::istream& mFile;
    
    std::uint16_t mFormat;
    std::uint16_t mNumDataFormats;
    std::uint16_t mFirstDataFormatID;
    std::uint32_t mInitOptionForChannel;
    std::uint16_t mNumSoundCommands;
    std::vector<std::uint64_t> mSoundCommands;

    std::unique_ptr<SoundSampleHeader> mSoundSampleHeader;
    std::unique_ptr<Decoder> mDecoder;

    std::vector<std::uint64_t> mSoundData; // Filled when interpreting bufferCmd.

    bool parse();
    bool decode();
    std::uint64_t findSoundCommand(std::uint16_t cmdName) const;
    bool doBufferCommand(std::uint64_t command);

    bool loadSoundSampleHeader(std::size_t offset);
    void loadSampleData(std::size_t offset, std::size_t sampleDataLength);
    void createDecompressionDecoder(const std::string& compressionFormatString,
        int compressionID);

public:
    static const unsigned char cStandardSoundHeaderEncode;
    static const unsigned char cExtendedSoundHeaderEncode;
    static const unsigned char cCompressedSoundHeaderEncode;

    SndFile(std::istream& file, const std::string& fileName);

    const SoundSampleHeader& getSoundSampleHeader() const;
    const Decoder& getDecoder() const;

    friend std::ostream& operator<<(std::ostream& lhs, const SndFile& rhs);
};

#endif // SND_FILE_HPP
