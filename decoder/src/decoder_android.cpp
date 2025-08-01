#include "decoder_android.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#include <GLES2/gl2.h>
#include <android/log.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>

//#include "imgui_impl_android.h"
//#include "imgui_impl_opengl3.h"
#include "logger.hpp"
#include "decoder_android_OMX.hpp"

#define CODEC_DIRECT_OUTPUT 1 // Enable decoder direct output to texture
#define CODEC_OUTPUT_FORMAT OMX_COLOR_FormatYCbYCr // Each 4 bytes represent two pixels: Y0 U0 Y1 V0
#define PIXEL_WIDTH 2 // YCbYCr format has 2 bytes per pixel
#define TEXTURE_FORMAT GL_ALPHA
#define TEXEL_WIDTH 1 // GL_ALPHA format has 1 byte per texel

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
                 TEXTURE_FORMAT, width, height * PIXEL_WIDTH, 0,
                 TEXTURE_FORMAT, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    textureID = (intptr_t)glTexture;
    return true;
}
//------------------------------------------------------------------------------
bool DecoderAndroid::UpdateTexture(const void* data)
{
    if (textureID == 0)
    {
        Log("Texture not created yet\n");
        return false;
    }
    if (data == nullptr)
    {
        Log("Data pointer is null\n");
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, (GLuint)textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height * PIXEL_WIDTH,
                    TEXTURE_FORMAT, GL_UNSIGNED_BYTE,
                    data); // Update texture with new data
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
    Log("Texture updated successfully\n");
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
            AMediaFormat_setInt32(mediaFormat, "color-format", CODEC_OUTPUT_FORMAT);
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
    ssize_t inputBufferIndex = AMediaCodec_dequeueInputBuffer(mediaCodec, 1000);
    if (inputBufferIndex >= 0)
    {
        size_t inputBufferSize;
        uint8_t* inputBuffer = AMediaCodec_getInputBuffer(mediaCodec, inputBufferIndex, &inputBufferSize);
        if (inputBuffer != nullptr && inputBufferSize > 0)
        {
            ssize_t size = AMediaExtractor_readSampleData(mediaExtractor, inputBuffer, inputBufferSize);
            if (size <= 0)
            {
                Log("Failed to read sample data\n");
                return false;
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
                media_status_t hr = AMediaCodec_queueInputBuffer(mediaCodec, inputBufferIndex,
                                                                 0, size, presentationTimeUs, 0);
                if (hr != AMEDIA_OK)
                {
                    Log("Failed to queue input buffer: %d\n", hr);
                    return false;
                }
                Log("Queue Input : AMEDIA_OK\n");
                Log("PTS : %lld\n", presentationTimeUs);
                Log("Size : %zu\n", size);

                // Advance the extractor to the next sample
                bool hasNext = AMediaExtractor_advance(mediaExtractor);
                if (!hasNext)
                {
                    Log("End of stream\n");
                    ssize_t emptyIndex = AMediaCodec_dequeueInputBuffer(mediaCodec, 1000);
                    AMediaCodec_queueInputBuffer(mediaCodec, emptyIndex, 0, 0, 0,
                                                 AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM );
                }
            }
        }
        else
        {
            Log("Failed to get input buffer\n");
            return false;
        }
    }
    else
    {
        Log("No input buffer available\n");
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
        Log("Output buffer rendered successfully\n");
        return true;
    }
    else if (outputBufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
    {
        Log("Output format changed\n");
        struct AMediaFormat* newFormat = AMediaCodec_getOutputFormat(mediaCodec);
        if (newFormat)
        {
            AMediaFormat_getInt32(newFormat, "width", &width);
            AMediaFormat_getInt32(newFormat, "height", &height);
            Log("New output format: width=%d, height=%d\n", width, height);
            AMediaFormat_delete(newFormat);
            if (textureID == 0)
                CreateTexture(); // Create texture if not already created
            else
                UpdateTexture(nullptr); // Update texture size if already created
            return true;
        }
        else
        {
            Log("Failed to get new output format\n");
            return false;
        }
    }
    else if (outputBufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER)
    {
        Log("Try again later\n");
        return true;
    }
    else
    {
        Log("Failed to dequeue output buffer: %zd\n", outputBufferIndex);
    }
    return false;
}