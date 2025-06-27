#pragma once

#include <cstdint>

class Decoder
{
public:
    explicit Decoder(class Logger& logger);
    virtual ~Decoder();

    virtual bool ReadMedia(const char* filePath) { return false; }
    virtual bool CreateTexture(const void* data) { return false; }
    virtual bool UpdateTexture(const void* data) { return false; }
    virtual void DestroyTexture() { }
    virtual bool CreateCodec() { return false; }
    virtual void DestroyCodec() { }
    virtual bool DecodeFrame() { return false; }
    virtual bool RenderFrame() { return false; }

    int width = 64;
    int height = 64;
    intptr_t textureID = 0;

protected:
    void Log(const char* fmt, ...);
    class Logger& logger_;
};