#pragma once

#include "decoder_ui.hpp"
#include <cstdint>

class Decoder
{
public:
    using LogFunc = DecoderUI::LogFunc;

    explicit Decoder(LogFunc ui_logger);
    virtual ~Decoder();

    virtual bool ReadMedia(const char* filePath, char* outMediaInfo, size_t infoSize) { return false; }
    virtual bool CreateTexture(const void* data) { return false; }
    virtual void DestroyTexture() { }
    virtual bool CreateCodec() { return false; }
    virtual void DestroyCodec() { }
    virtual bool Decode() { return false; }
    virtual bool Render() { return false; }

    int width = 64;
    int height = 64;
    intptr_t textureID = 0;

protected:
    LogFunc Log = nullptr;
};