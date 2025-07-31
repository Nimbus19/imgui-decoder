#pragma once

#include "decoder.hpp"

class DecoderApple : public Decoder
{
public:
    explicit DecoderApple(Logger ui_logger);
    virtual ~DecoderApple();

    bool ReadMedia(const char* filePath, char* outMediaInfo, size_t infoSize) override;
};