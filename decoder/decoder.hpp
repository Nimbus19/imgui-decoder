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

    void DrawUI();
    bool ReadFile();

    static GLuint CreateTexture(int width, int height, const void* data = nullptr);
    static void DestroyTexture(GLuint textureID);

private:
    struct android_app* m_App;
    GLuint textureID = 0;

    static void Log(const char* fmt, ...);
};