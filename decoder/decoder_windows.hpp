#pragma once

#include "decoder.hpp"
#include <windows.h>

class DecoderWindows : public Decoder
{
public:
    explicit DecoderWindows(class Logger& logger, struct ID3D11Device* d3d_device,
        struct ID3D11DeviceContext* d3d_context, struct IDXGISwapChain* d3d_swapchain);
    virtual ~DecoderWindows();

    bool ReadMedia(const char* file_path) override;
    bool CreateTexture(const void* data) override;
    void DestroyTexture() override;
    bool CreateCodec() override;
    void DestroyCodec() override;
    bool DecodeFrame() override;
    bool RenderFrame() override;

private:
    void PrintMediaType(struct IMFMediaType* type);
    const char* GuidToName(const GUID& guid);
    void SetOutputType();
    void AllocateOutputSample();

    struct ID3D11Device* d3d_device_ = nullptr;
    struct ID3D11DeviceContext* d3d_context_ = nullptr;
    struct IDXGISwapChain* d3d_swapchain_ = nullptr;

    struct IMFSourceReader* reader_ = nullptr;
    struct IMFMediaType* input_type_ = nullptr;
    struct IMFMediaType* output_type_ = nullptr;
    struct IMFTransform* codec_ = nullptr;
    struct IMFSample* output_sample_ = nullptr;
    struct IMFMediaBuffer* output_buffer_ = nullptr;
    DWORD steam_id_ = 0;
};