#include "decoder.hpp"
#include "imgui.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#if defined(__ANDROID__)
#   include <android/log.h>
#   include <media/NdkMediaCodec.h>
#   include <media/NdkMediaExtractor.h>
#endif

//------------------------------------------------------------------------------
Decoder::Decoder(android_app* app, LogFunc logFunc)
{
    app_ = app;
    log_func_ = logFunc;
}
//------------------------------------------------------------------------------
Decoder::~Decoder()
{
    DestroyTexture(textureID);
}
//------------------------------------------------------------------------------
bool Decoder::ReadFile(const char* filePath, char* outBuffer, size_t bufferSize)
{
    FILE* file = fopen(filePath, "r");
    if (file == nullptr)
    {
        sprintf(outBuffer, "Failed to open file: %s", filePath);
        return false;
    }

    fgets(outBuffer, (int)bufferSize, file);

    fclose(file);
    return true;
}
//------------------------------------------------------------------------------
GLuint Decoder::CreateTexture(int width, int height, const void* data)
{
    // create texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // send data to GPU memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    return textureID;
}
//------------------------------------------------------------------------------
void Decoder::DestroyTexture(GLuint& textureID)
{
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}
//------------------------------------------------------------------------------
bool Decoder::GetMediaFormat(AMediaFormat*& outMediaFormat, const char* url, const char* mine)
{
//    FILE *fp;
//    fp = fopen(url, "r");
//    if (fp == nullptr)
//        return false;
//
//    int fd = fileno(fp);
//    fseek(fp, 0, SEEK_END);
//    off64_t size = ftell(fp);
//
//    bool result = false;
//    switch (0) case 0: default:
//    {
//        AMediaExtractor *mediaExtractor = AMediaExtractor_new();
//        media_status_t hr = AMediaExtractor_setDataSourceFd(mediaExtractor, fd, 0, size);
//        size_t trackCount = AMediaExtractor_getTrackCount(mediaExtractor);
//
//        if (hr != AMEDIA_OK or trackCount == 0)
//        {
//            result = false;
//            break;
//        }
//
//        for (int i = 0; i < trackCount; i++)
//        {
//            AMediaFormat* trackFormat = AMediaExtractor_getTrackFormat(mediaExtractor, i);
//            const char* trackMine = "";
//            AMediaFormat_getString(trackFormat, "mime", &trackMine);
//
//            if (strncmp(mine, trackMine, strlen(mine)) == 0)
//            {
//                AMediaFormat_delete(outMediaFormat);
//                outMediaFormat = trackFormat;
//                result = true;
//                break;
//            }
//
//            AMediaFormat_delete(trackFormat);
//        }
//    }
//
//    fclose(fp);
//    return result;
    return false;
}
//------------------------------------------------------------------------------
void Decoder::Log(const char *fmt, ...)
{
    if (log_func_)
    {
        va_list args;
        va_start(args, fmt);
        log_func_(fmt, args);
        va_end(args);
    }
}