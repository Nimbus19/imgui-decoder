#pragma once

#include <cstdint>

class Decoder
{
public:
    explicit Decoder(class Logger* logger);
    virtual ~Decoder();

    virtual bool ReadMedia(const char*) { return false; }
    virtual bool CreateTexture() { return false; }
    virtual bool UpdateTexture(const void*) { return false; }
    virtual void DestroyTexture() { }
    virtual bool CreateCodec() { return false; }
    virtual void DestroyCodec() { }
    virtual bool DecodeFrame() { return false; }
    virtual bool RenderFrame() { return false; }

    int video_width = 64;
    int video_height = 64;
    int texture_width = 64;
    int texture_height = 64;
    intptr_t textureID = 0;

protected:
    void Log(const char* fmt, ...);

    class Logger* logger_ = nullptr;
    bool should_release_logger_ = false;
};