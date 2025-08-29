#include "decoder_android_texture.hpp"

#include "logger.hpp"

#define TEXTURE_FORMAT GL_LUMINANCE

//------------------------------------------------------------------------------
AndroidTexture::AndroidTexture(Logger* logger, android_app* g_app)
{
    logger_ = logger;
    app_ = g_app;
}
//------------------------------------------------------------------------------
AndroidTexture::~AndroidTexture()
{
    DestroyGLTexture();
    DestroyOESTexture();
}
//------------------------------------------------------------------------------
bool AndroidTexture::CreateGLTexture(int width, int height)
{
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 TEXTURE_FORMAT, width, height, 0,
                 TEXTURE_FORMAT, GL_UNSIGNED_BYTE, nullptr);

    return true;
}
//------------------------------------------------------------------------------
void AndroidTexture::DestroyGLTexture()
{
    if (glTex != 0)
    {
        glDeleteTextures(1, &glTex);
        glTex = 0;
    }
}
//------------------------------------------------------------------------------
bool AndroidTexture::UpdateGLTexture(int width, int height, const void* data)
{
    if (glTex == 0)
    {
        logger_->Log("Texture not created yet\n");
        return false;
    }
    if (data == nullptr)
    {
        logger_->Log("Data pointer is null\n");
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, glTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    width, height,
                    TEXTURE_FORMAT, GL_UNSIGNED_BYTE,
                    data); // Update texture with new data
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    return true;
}
//------------------------------------------------------------------------------
bool AndroidTexture::CreateOESTexture()
{
    glGenTextures(1, &oesTex);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, oesTex);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}
//------------------------------------------------------------------------------
void AndroidTexture::DestroyOESTexture()
{
    if (oesTex != 0)
    {
        glDeleteTextures(1, &oesTex);
        oesTex = 0;
    }
}
