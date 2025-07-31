#pragma once

#include <cstdio>
#include "decoder.hpp"

class DecoderAndroid : public Decoder
{
public:
    explicit DecoderAndroid(class Logger* ui_logger, struct android_app* g_app);
    virtual ~DecoderAndroid();

    bool ReadMedia(const char* filePath) override;
    bool CreateTexture() override;
    void DestroyTexture() override;
    bool CreateCodec() override;
    void DestroyCodec() override;

private:
    struct android_app* app_ = nullptr;
    struct AMediaFormat* mediaFormat = nullptr;
    struct AMediaCodec* mediaCodec = nullptr;

    bool GetMediaFormat(struct AMediaFormat*& outMediaFormat, FILE* file);
    struct AMediaCodec* createMediaCodec(bool isHardware, const char* codecString, struct AMediaFormat* format);

};