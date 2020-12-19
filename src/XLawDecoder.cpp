/* Copyright (c) 2020 Iliyas Jorio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// a-law and mu-law decoder.
// By jorio for Pomme (https://github.com/jorio/Pomme).
// Modified by Carl Hewett for SndToWAV.

#include "XLawDecoder.hpp"
#include <iostream>
#include <cstddef> // For size_t

// Conversion tables to obtain 16-bit PCM from 8-bit a-law/mu-law.
// These tables are valid for *-law input bytes [0...127].
// For inputs [-127...-1], mirror the table and negate the output.
//
// These tables were generated using alaw2linear() and ulaw2linear()
// from ffmpeg/libavcodec/pcm_tablegen.h.

static const std::int16_t aLawToPCM[128] = {
 -5504,  -5248,  -6016,  -5760,  -4480,  -4224,  -4992,  -4736,
 -7552,  -7296,  -8064,  -7808,  -6528,  -6272,  -7040,  -6784,
 -2752,  -2624,  -3008,  -2880,  -2240,  -2112,  -2496,  -2368,
 -3776,  -3648,  -4032,  -3904,  -3264,  -3136,  -3520,  -3392,
-22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944,
-30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136,
-11008, -10496, -12032, -11520,  -8960,  -8448,  -9984,  -9472,
-15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568,
  -344,   -328,   -376,   -360,   -280,   -264,   -312,   -296,
  -472,   -456,   -504,   -488,   -408,   -392,   -440,   -424,
   -88,    -72,   -120,   -104,    -24,     -8,    -56,    -40,
  -216,   -200,   -248,   -232,   -152,   -136,   -184,   -168,
 -1376,  -1312,  -1504,  -1440,  -1120,  -1056,  -1248,  -1184,
 -1888,  -1824,  -2016,  -1952,  -1632,  -1568,  -1760,  -1696,
  -688,   -656,   -752,   -720,   -560,   -528,   -624,   -592,
  -944,   -912,  -1008,   -976,   -816,   -784,   -880,   -848,
};

static const std::int16_t uLawToPCM[128] = {
-32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
-23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
-15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
-11900, -11388, -10876, -10364,  -9852,  -9340,  -8828,  -8316,
 -7932,  -7676,  -7420,  -7164,  -6908,  -6652,  -6396,  -6140,
 -5884,  -5628,  -5372,  -5116,  -4860,  -4604,  -4348,  -4092,
 -3900,  -3772,  -3644,  -3516,  -3388,  -3260,  -3132,  -3004,
 -2876,  -2748,  -2620,  -2492,  -2364,  -2236,  -2108,  -1980,
 -1884,  -1820,  -1756,  -1692,  -1628,  -1564,  -1500,  -1436,
 -1372,  -1308,  -1244,  -1180,  -1116,  -1052,   -988,   -924,
  -876,   -844,   -812,   -780,   -748,   -716,   -684,   -652,
  -620,   -588,   -556,   -524,   -492,   -460,   -428,   -396,
  -372,   -356,   -340,   -324,   -308,   -292,   -276,   -260,
  -244,   -228,   -212,   -196,   -180,   -164,   -148,   -132,
  -120,   -112,   -104,    -96,    -88,    -80,    -72,    -64,
   -56,    -48,    -40,    -32,    -24,    -16,     -8,     -0,
};

/* XLawDecoder */
XLawDecoder::XLawDecoder()
{
    // Do nothing
}

std::size_t XLawDecoder::getEncodedSize(std::size_t numPackets) const
{
    return numPackets * 1; // 1 byte per packet.
}

std::size_t XLawDecoder::getDecodedSize(std::size_t numPackets) const
{
    // Each packets decodes into a 2-byte sample.
    return numPackets * 2;
}

unsigned XLawDecoder::getBitsPerSample() const
{
    return 16;
}

bool XLawDecoder::decode(const std::vector<std::uint8_t>& data,
    std::size_t /* numChannels */, bool useULaw)
{
    const std::int16_t* xLawToPCM = aLawToPCM; // Get correct table
    std::vector<std::int16_t> decodedSamples(data.size());

    if(useULaw)
        xLawToPCM = uLawToPCM;

    for(std::size_t i = 0; i < data.size(); i++)
    {
        std::int8_t b = *reinterpret_cast<const std::int8_t*>(&data[i]); // SIGNED!
        if(b < 0)
        {
            // Mirror table and negate output
            decodedSamples[i] = -xLawToPCM[128 + b];
        } else
        {
            decodedSamples[i] = xLawToPCM[b];
        }
    }

    setLittleEndianData(decodedSamples);
    return true;
}

/* ALawDecoder */
ALawDecoder::ALawDecoder()
{
    // Do nothing
}

bool ALawDecoder::decode(const std::vector<std::uint8_t>& data,
    std::size_t numChannels)
{
    return XLawDecoder::decode(data, numChannels, false);
}

/* ULawDecoder */
ULawDecoder::ULawDecoder()
{
    // Do nothing
}

bool ULawDecoder::decode(const std::vector<std::uint8_t>& data,
    std::size_t numChannels)
{
    return XLawDecoder::decode(data, numChannels, true);
}

