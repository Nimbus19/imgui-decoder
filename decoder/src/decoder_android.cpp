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
    if (mediaExtractor != nullptr)
    {
        AMediaExtractor_delete(mediaExtractor);
        mediaExtractor = nullptr;
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
    GetMediaFormat(file);
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
bool DecoderAndroid::GetMediaFormat(FILE* file)
{
    if (file == nullptr)
    {
        Log("File pointer is null\n");
        return false;
    }
    if (mediaExtractor != nullptr)
    {
        AMediaExtractor_delete(mediaExtractor);
        mediaExtractor = nullptr;
    }
    if (mediaFormat != nullptr)
    {
        AMediaFormat_delete(mediaFormat);
        mediaFormat = nullptr;
    }

    int fd = fileno(file);
    fseek(file, 0, SEEK_END);
    off64_t size = ftell(file);

    mediaExtractor = AMediaExtractor_new();
    media_status_t hr = AMediaExtractor_setDataSourceFd(mediaExtractor, fd, 0, size);

    if (hr != AMEDIA_OK)
    {
        Log("Failed to set data source: %d\n", hr);
        AMediaExtractor_delete(mediaExtractor);
        mediaExtractor = nullptr;
        return false;
    }

    size_t trackCount = AMediaExtractor_getTrackCount(mediaExtractor);
    if (trackCount == 0)
    {
        Log("No tracks found in media file\n");
        AMediaExtractor_delete(mediaExtractor);
        mediaExtractor = nullptr;
        return false;
    }

    for (int i = 0; i < trackCount; i++)
    {
        AMediaFormat* trackFormat = AMediaExtractor_getTrackFormat(mediaExtractor, i);
        const char* trackMine = "";
        AMediaFormat_getString(trackFormat, "mime", &trackMine);

        Log("Track %d: %s\n", i, trackMine);

        if (strncmp("video/", trackMine, 6) == 0)
        {
            mediaFormat = trackFormat;
            AMediaExtractor_selectTrack(mediaExtractor, i);
        }
        else
        {
            AMediaFormat_delete(trackFormat);
        }
    }
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
//------------------------------------------------------------------------------
bool DecoderAndroid::DecodeFrame()
{
    if (mediaCodec == nullptr || mediaFormat == nullptr)
        return false;

    AMediaCodecBufferInfo info;
    media_status_t hr = AMEDIA_ERROR_UNKNOWN;
    ssize_t inputBufferIndex = AMediaCodec_dequeueInputBuffer(mediaCodec, 1000);
    if (inputBufferIndex >= 0)
    {
        size_t inputBufferSize;
        uint8_t* inputBuffer = AMediaCodec_getInputBuffer(mediaCodec, inputBufferIndex, &inputBufferSize);
        if (inputBuffer != nullptr && inputBufferSize > 0)
        {
            ssize_t size = AMediaExtractor_readSampleData(mediaExtractor, inputBuffer, inputBufferSize);
            if (size < 0)
            {
                Log("Failed to read sample data\n");
                return false;
            }
            if (size == 0)
            {
                // End of stream
//                hr = AMediaCodec_signalEndOfInputStream(mediaCodec);
//                if (hr != AMEDIA_OK)
//                {
//                    Log("Failed to signal end of input stream: %d\n", hr);
//                    return false;
//                }
                Log("End of input stream signaled\n");
            }
            else
            {
                // Set the presentation timestamp
                int64_t presentationTimeUs = AMediaExtractor_getSampleTime(mediaExtractor);
                if (presentationTimeUs < 0)
                {
                    Log("Failed to get sample time\n");
                    return false;
                }
                // Queue the input buffer with the data read from the extractor
                hr = AMediaCodec_queueInputBuffer(mediaCodec, inputBufferIndex, 0, size, presentationTimeUs, 0);
                if (hr != AMEDIA_OK)
                {
                    Log("Failed to queue input buffer: %d\n", hr);
                    return false;
                }
                Log("Input buffer queued successfully, size: %zu, PTS: %lld\n", size, presentationTimeUs);
            }
        }
    }

    return true;
}
//------------------------------------------------------------------------------
bool DecoderAndroid::RenderFrame()
{
    if (mediaCodec == nullptr || mediaFormat == nullptr)
        return false;

    AMediaCodecBufferInfo info;
    ssize_t outputBufferIndex = AMediaCodec_dequeueOutputBuffer(mediaCodec, &info, 1000);
    if (outputBufferIndex >= 0)
    {
        size_t outputBufferSize;
        uint8_t* outputBuffer = AMediaCodec_getOutputBuffer(mediaCodec, outputBufferIndex, &outputBufferSize);
        if (outputBuffer != nullptr && outputBufferSize > 0)
        {
            // Process the output buffer
            UpdateTexture(outputBuffer); // Update texture with decoded frame data
        }
        AMediaCodec_releaseOutputBuffer(mediaCodec, outputBufferIndex, true);
    }
    else if (outputBufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
    {
        Log("Output format changed\n");
    }
    else if (outputBufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER)
    {
        Log("No output available\n");
    }
    return true;
}