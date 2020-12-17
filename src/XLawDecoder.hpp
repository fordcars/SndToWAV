#ifndef XLAW_DECODER_HPP
#define XLAW_DECODER_HPP

#include <vector>
#include <cstdint>

class XLawDecoder : public Decoder
{
protected:
    XLawDecoder();

    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels, bool useULaw);
};

class ALawDecoder : public XLawDecoder
{
public:
    ALawDecoder();
    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels);
};


class ULawDecoder : public XLawDecoder
{
public:
    ULawDecoder();
    std::vector<std::int16_t> decode(const std::vector<std::uint8_t>& data,
        unsigned numChannels) override;
};

#endif // XLAW_DECODER_HPP
