#pragma once

#include "decoder.hpp"

class DecoderAndroid : public Decoder
{
public:
    explicit DecoderAndroid(Logger ui_logger);
    virtual ~DecoderAndroid();

    bool ReadMedia(const char* filePath, char* outMediaInfo, size_t infoSize) override;
    bool CreateTexture(const void* data) override;
    void DestroyTexture() override;    

private:
    struct android_app* app_ = nullptr;

    static bool GetMediaFormat(struct AMediaFormat*& outMediaFormat, const char* url, const char* mine);
};