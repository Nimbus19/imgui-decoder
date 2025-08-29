#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class AndroidTexture
{
public:
    explicit AndroidTexture(class Logger* logger, struct android_app* g_app);
    virtual ~AndroidTexture();

    bool CreateGLTexture(int width, int height);
    void DestroyGLTexture();
    bool UpdateGLTexture(int width, int height, const void* data);

    bool CreateOESTexture();
    void DestroyOESTexture();

    GLuint glTex = 0;
    GLuint oesTex = 0;

private:
    struct android_app* app_ = nullptr;
    class Logger* logger_ = nullptr;
};