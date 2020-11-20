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

#include "IMA4Decoder.hpp"

#include "SndToWAV.hpp"

#include <cstddef> // For std::size_t
#include <cmath> // For floor()
#include <iostream>
#include <limits> // For numeric limits

namespace
{
    // From https://web.archive.org/web/20111117212301/http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
    int gIndexTable[16] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    int gStepTable[89] = { 
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
         2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
    };
}

IMA4Decoder::IMA4Decoder()
{
}

// Returns the native-endian uncompressed sample for this nibble.
// Nibble only uses the lower 4-bits.
// From https://web.archive.org/web/20111026200128/http://www.wooji-juice.com/blog/iphone-openal-ima4-adpcm.html
std::int16_t IMA4Decoder::processNibble(std::uint8_t nibble)
{
    // Select lower 4-bits only, for safety.
    nibble = nibble & 0x0f;

    // Nibbles have a sign-magnitude representation!
    // See p.6: http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf
    std::uint8_t nibbleMagnitude = nibble & 0x07;
    bool isNegative = (nibble & 0x08) == 0x08;

    // Get step.
    int step = gStepTable[mStepIndex];

    // In the specs, the formula mentionned is:
    //     ((signed)nibble + 0.5) * step / 4
    // which yields erroneous results. Instead, we use this:
    //     ((signed)((unsigned)nibble + 0.5)) * step / 4
    int diff = static_cast<int>(
        (nibbleMagnitude + 0.5f) * step/4.0f
    );
    if(isNegative)
        diff = -diff;

    // Calculate new predictor (sample).
    mPredictor += diff;
    // Clamp predictor.
    mPredictor = clamp(
        std::numeric_limits<std::int16_t>::min(),
        mPredictor,
        std::numeric_limits<std::int16_t>::max()
    );

    // Get next step index.
    mStepIndex += gIndexTable[nibble];
    // Clamp step index according to step table size:
    mStepIndex = clamp(0, mStepIndex, 88);

    return static_cast<std::int16_t>(mPredictor);
}

// Returns native-endian signed values.
// Based on:
// https://web.archive.org/web/20111026200128/http://www.wooji-juice.com/blog/iphone-openal-ima4-adpcm.html
// https://web.archive.org/web/20111117212301/http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
// http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf
// https://wiki.multimedia.cx/index.php/Apple_QuickTime_IMA_ADPCM
// Answers by Laurent Etiemble and Arthur Shipkowski from:
// --- https://stackoverflow.com/questions/2130831/decoding-ima4-audio-format
std::vector<std::int16_t> IMA4Decoder::decodeFrame(const std::uint8_t frame[IMA4_PACKET_LENGTH])
{
    // Header is first 2 Big-endian bytes.
    std::uint16_t header = SndToWAV::makeBigEndianNative(
        *reinterpret_cast<const std::uint16_t*>(frame)
    );

    std::vector<std::int16_t> decodedSamples;

    mStepIndex = header & 0x007f; // Lower 7 bits.
    mStepIndex = clamp(0, mStepIndex, 88); // Clamp for good measure (7 bits is 0..127, we want 0..88).

    // Upper 9 bits. Represents the top 9 bits of the 16-bit value, so sign is important!
    mPredictor = castSigned16Bit<int>(header & 0xff80);

    // Iterate through all bytes of frame, starting after the header.
    for(std::size_t i = 2; i < IMA4_PACKET_LENGTH; ++i)
    {
        // Make sure frame[i] is unsigned to avoid sign-extension problems.
        std::uint8_t lowNibble = frame[i] & 0x0f; // Lower 4 bits.
        std::uint8_t highNibble = frame[i] >> 4; // Top 4 bits.

        // We must process low nibble first, then high nibble!
        // These are little-endian values.
        std::int16_t sample1 = processNibble(lowNibble);
        std::int16_t sample2 = processNibble(highNibble);

        decodedSamples.push_back(sample1);
        decodedSamples.push_back(sample2);
    }

    return decodedSamples;
}

// Returns interweaved native-endian signed samples.
std::vector<std::int16_t> IMA4Decoder::decodeStereoFrame(const std::uint8_t leftFrame[IMA4_PACKET_LENGTH],
        const std::uint8_t rightFrame[IMA4_PACKET_LENGTH])
{
    std::vector<std::int16_t> leftSamples = decodeFrame(leftFrame);
    std::vector<std::int16_t> rightSamples = decodeFrame(rightFrame);

    // Make enough space for both channels.
    // Both left and right samples should be the same length (32*2 bytes).
    std::vector<std::int16_t> stereoSamples(leftSamples.size() + rightSamples.size(), 0);

    // Interweave.
    for(std::size_t i = 0; i < leftSamples.size(); ++i)
    {
        // Left channel is first!
        stereoSamples[i*2] = leftSamples[i];
        stereoSamples[i*2 + 1] = rightSamples[i];
    }

    return stereoSamples;
}

// Returns the native-endian decoded sound samples (interleaved if stereo).
std::vector<std::int16_t> IMA4Decoder::decode(const std::vector<std::uint8_t>& data,
    unsigned numChannels)
{
    std::vector<std::int16_t> decodedSamples;

    if(data.size() % 34 != 0)
    {
        std::cout << "Warning: data given to IMA4 decoder is not a multiple of 34 bytes! " <<
            "Is this truly IMA4 data?" << std::endl;
    } else if(numChannels == 2 && data.size() % (34*2) != 0)
    {
        std::cout << "Warning: stereo data given to IMA4 decoder is not a multiple " <<
            "of 34 * 2 bytes! Is this truly stereo IMA4 data?" << std::endl;
    }

    if(numChannels != 1 && numChannels != 2)
    {
        std::cerr << "Error: invalid number of channels given (" << numChannels << "). " <<
            " Can only decode IMA4 with 1 (mono) or 2 (stereo) channels!" << std::endl;
        return decodedSamples;
    }

    if(numChannels == 1)
    {
        // Iterate through every 34-byte packet.
        for(long long int i = 0;
            i < static_cast<long long int>(data.size()) - IMA4_PACKET_LENGTH;
            i += IMA4_PACKET_LENGTH)
        {
            std::vector<std::int16_t> samples = decodeFrame(&data[i]);

            // From https://stackoverflow.com/questions/2551775/appending-a-vector-to-a-vector
            decodedSamples.insert(decodedSamples.end(), samples.begin(), samples.end());
        }
    } else if(numChannels == 2)
    {
        // Iterate through every pair of 34-byte packets.
        for(long long int i = 0;
            i < static_cast<long long int>(data.size()) - IMA4_PACKET_LENGTH*2;
            i += IMA4_PACKET_LENGTH*2)
        {
            std::vector<std::int16_t> stereoSamples = decodeStereoFrame(&data[i],
                &data[i + IMA4_PACKET_LENGTH]);

            // From https://stackoverflow.com/questions/2551775/appending-a-vector-to-a-vector
            decodedSamples.insert(decodedSamples.end(), stereoSamples.begin(), stereoSamples.end());
        }
    }

    return decodedSamples;
}
