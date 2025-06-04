#pragma once

#   include <EGL/egl.h>
#   include <GLES3/gl3.h>

class Decoder
{
public:
    Decoder(struct android_app* app);
    virtual ~Decoder();

    const int width = 64;
    const int height = 64;

    char outputPath[256] = "";
    char prefix[64] = "output";
    int exportFormat = 0; // 0: Mp4, 1: M4e, 2: Mp4 & M4e

    void DrawUI();
    bool ReadFile();

    static GLuint CreateTexture(int width, int height, const void* data = nullptr);
    static void DestroyTexture(GLuint textureID);
    static bool GetMediaFormat(struct AMediaFormat*& outMediaFormat, const char* url, const char* mine);
private:
    struct android_app* m_App;
    GLuint textureID = 0;

    static void Log(const char* fmt, ...);
};