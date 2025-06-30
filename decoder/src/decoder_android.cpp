#include "decoder_android.hpp"
#include "imgui.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#include <android/log.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>

//------------------------------------------------------------------------------
DecoderAndroid::DecoderAndroid(Decoder::Logger ui_logger)
    : Decoder(ui_logger)
{
    app_ = nullptr;
}
//------------------------------------------------------------------------------
DecoderAndroid::~DecoderAndroid()
{
    DestroyTexture();
}
//------------------------------------------------------------------------------
bool DecoderAndroid::ReadMedia(const char* filePath, char* outMediaInfo, size_t infoSize)
{
    FILE* file = fopen(filePath, "r");
    if (file == nullptr)
    {
        sprintf(outMediaInfo, "Failed to open file: %s", filePath);
        return false;
    }

    for (size_t i = 0; i < (infoSize / 3) - 1; i += 3)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            outMediaInfo[i] = '\0'; // Null-terminate the string
            break;
        }
        sprintf(outMediaInfo + i, "%02X ", c);
    }
    return true;
}
//------------------------------------------------------------------------------
bool DecoderAndroid::CreateTexture(const void* data)
{
    // create texture
    GLuint glTexture;
    glGenTextures(1, &glTexture);
    glBindTexture(GL_TEXTURE_2D, glTexture);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // send data to GPU memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    textureID = (intptr_t)glTexture;
    return true;
}
//------------------------------------------------------------------------------
void DecoderAndroid::DestroyTexture()
{
    if (textureID != 0)
    {
        glDeleteTextures(1, (GLuint*)&textureID);
        textureID = 0;
    }
}
//------------------------------------------------------------------------------
bool DecoderAndroid::GetMediaFormat(AMediaFormat*& outMediaFormat, const char* url, const char* mine)
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
