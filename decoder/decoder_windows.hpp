#pragma once

#include "decoder.hpp"
#include <windows.h>

class DecoderWindows : public Decoder
{
public:
    explicit DecoderWindows(LogFunc ui_logger, struct ID3D11Device* d3d_device, struct ID3D11DeviceContext* d3d_context, struct IDXGISwapChain* d3d_swapchain);
    virtual ~DecoderWindows();

    bool ReadMedia(const char* file_path, char* media_info, size_t info_size) override;
    bool CreateTexture(const void* data) override;
    void DestroyTexture() override;
    bool CreateCodec() override;
    void DestroyCodec() override;

private:
    struct ID3D11Device* d3d_device_;
    struct ID3D11DeviceContext* d3d_context_;
    struct IDXGISwapChain* d3d_swapchain_;

    struct IMFSourceReader* pReader = nullptr;
    struct IMFMediaType* pMediaType = nullptr;
    struct IMFMediaType* pOutputMediaType = nullptr;
    struct IMFTransform* pTransform = nullptr;
    struct IMFSample* outputSample = nullptr;
    struct IMFMediaBuffer* outputBuffer = nullptr;
    DWORD streamID = 0;
};