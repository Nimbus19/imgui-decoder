#include "decoder_android.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#include <GLES2/gl2.h>
#include <android/log.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>

//#include "imgui_impl_android.h"
//#include "imgui_impl_opengl3.h"
#include "logger.hpp"

//------------------------------------------------------------------------------
DecoderAndroid::DecoderAndroid(Logger* ui_logger, android_app* g_app)
    : Decoder(ui_logger)
{
    app_ = g_app;
}
//------------------------------------------------------------------------------
DecoderAndroid::~DecoderAndroid()
{
    if (mediaFormat != nullptr)
    {
        AMediaFormat_delete(mediaFormat);
        mediaFormat = nullptr;
    }
    DestroyCodec();
    DestroyTexture();
}
//------------------------------------------------------------------------------
bool DecoderAndroid::ReadMedia(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    if (file == nullptr)
    {
        Log("Failed to open file: %s\n", filePath);
        return false;
    }
    GetMediaFormat(mediaFormat, file);
    fclose(file);
    return true;
}
//------------------------------------------------------------------------------
bool DecoderAndroid::CreateTexture()
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

    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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
bool DecoderAndroid::CreateCodec()
{
    DestroyCodec();

    mediaCodec = createMediaCodec(true, "video/avc", mediaFormat);
    if (mediaCodec == nullptr)
    {
        Log("Failed to create codec\n");
        return false;
    }
    Log("Codec created successfully\n");

    struct AMediaFormat* format = AMediaCodec_getOutputFormat(mediaCodec);
    if (format)
    {
        int videoMediaFormatID;
        AMediaFormat_getInt32(format, "color-format", &videoMediaFormatID);
        Log("Video Media Format ID: %d\n", videoMediaFormatID);
        AMediaFormat_delete(format);
    }

    return true;
}
//------------------------------------------------------------------------------
void DecoderAndroid::DestroyCodec()
{
    if (mediaCodec != nullptr)
    {
        AMediaCodec_stop(mediaCodec);
        AMediaCodec_flush(mediaCodec);
        AMediaCodec_delete(mediaCodec);
        mediaCodec = nullptr;
    }
}
//------------------------------------------------------------------------------
bool DecoderAndroid::GetMediaFormat(AMediaFormat*& outMediaFormat, FILE* file)
{
    int fd = fileno(file);
    fseek(file, 0, SEEK_END);
    off64_t size = ftell(file);

    AMediaExtractor *mediaExtractor = AMediaExtractor_new();
    media_status_t hr = AMediaExtractor_setDataSourceFd(mediaExtractor, fd, 0, size);

    if (hr != AMEDIA_OK)
    {
        Log("Failed to set data source: %d\n", hr);
        AMediaExtractor_delete(mediaExtractor);
        return false;
    }

    size_t trackCount = AMediaExtractor_getTrackCount(mediaExtractor);
    if (trackCount == 0)
    {
        Log("No tracks found in media file\n");
        AMediaExtractor_delete(mediaExtractor);
        return false;
    }

    for (int i = 0; i < trackCount; i++)
    {
        AMediaFormat* trackFormat = AMediaExtractor_getTrackFormat(mediaExtractor, i);
        const char* trackMine = "";
        AMediaFormat_getString(trackFormat, "mime", &trackMine);

        Log("Track %d: %s\n", i, trackMine);

        if (strncmp("video/", trackMine, 6) == 0)
            outMediaFormat = trackFormat;
        else
            AMediaFormat_delete(trackFormat);
    }
    AMediaExtractor_delete(mediaExtractor);
    return true;
}
//------------------------------------------------------------------------------
AMediaCodec* DecoderAndroid::createMediaCodec(bool isHardware, const char* codecString, AMediaFormat* format)
{
    AMediaCodec* codec = nullptr;
    if (isHardware)
        codec = AMediaCodec_createDecoderByType(codecString);
    else
        codec = AMediaCodec_createCodecByName(codecString);

    if (codec == nullptr)
        return nullptr;

    media_status_t hr = AMediaCodec_configure(codec, mediaFormat, nullptr, nullptr, 0);
    if (hr != AMEDIA_OK)
    {
        Log("Failed to configure codec\n");
        AMediaCodec_delete(codec);
        return nullptr;
    }
    Log("Codec configured successfully\n");

    hr = AMediaCodec_start(codec);
    if (hr != AMEDIA_OK)
    {
        Log("Failed to start codec\n");
        AMediaCodec_delete(codec);
        return nullptr;
    }
    Log("Codec started successfully\n");

    return codec;
}
