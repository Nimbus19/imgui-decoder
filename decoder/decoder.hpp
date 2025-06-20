#pragma once

#   include <EGL/egl.h>
#   include <GLES3/gl3.h>

class Decoder
{
public:
    using LogFunc = void(*)(const char*, ...);

    explicit Decoder(struct android_app* app, LogFunc logFunc);
    virtual ~Decoder();

    static GLuint CreateTexture(int width, int height, const void* data = nullptr);
    static void DestroyTexture(GLuint& textureID);
    static bool GetMediaFormat(struct AMediaFormat*& outMediaFormat, const char* url, const char* mine);
    bool ReadFile();

    const int width = 64;
    const int height = 64;
    GLuint textureID = 0;

private:
    void Log(const char* fmt, ...);

    struct android_app* app_ = nullptr;
    LogFunc log_func_ = nullptr;
};