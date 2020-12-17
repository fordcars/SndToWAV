#ifndef XLAW_DECODER_HPP
#define XLAW_DECODER_HPP

#include <vector>
#include <cstdint>

class XLawDecoder
{
protected:
    XLawDecoder();

    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels, bool useULaw);
};

class ALawDecoder : private XLawDecoder
{
public:
    ALawDecoder();
    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels);
};


class ULawDecoder : private XLawDecoder
{
public:
    ULawDecoder();
    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels);
};

#endif // XLAW_DECODER_HPP
