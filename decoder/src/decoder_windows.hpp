#pragma once

#include <windows.h>
#include "decoder.hpp"

class DecoderWindows : public Decoder
{
public:
    explicit DecoderWindows(class Logger* logger, struct ID3D11Device* d3d_device,
        struct ID3D11DeviceContext* d3d_context, struct IDXGISwapChain* d3d_swapchain);
    virtual ~DecoderWindows();

    bool ReadMedia(const char* file_path) override;
    bool CreateTexture() override;
    bool UpdateTexture(const void* data) override;
    void DestroyTexture() override;
    bool CreateCodec() override;
    void DestroyCodec() override;
    bool DecodeFrame() override;
    bool RenderFrame() override;

    struct ID3D11ShaderResourceView* CreateSRV();
    void DestroySRV();

    struct ID3D11ShaderResourceView* srv = nullptr;

private:
    template <class T> void SafeRelease(T** ppT);
    void PrintWinError(const char* funcName, HRESULT hr);
    void PrintMediaType(struct IMFMediaType* type);
    const char* GuidToName(const GUID& guid);
    bool SetOutputType();
    bool AllocateOutputSample();

    struct ID3D11Device* d3d_device_ = nullptr;
    struct ID3D11DeviceContext* d3d_context_ = nullptr;
    struct IDXGISwapChain* d3d_swapchain_ = nullptr;
    struct ID3D11Texture2D* d3d_texture_ = nullptr;

    struct IMFSourceReader* reader_ = nullptr;
    struct IMFMediaType* input_type_ = nullptr;
    struct IMFMediaType* output_type_ = nullptr;
    struct IMFTransform* codec_ = nullptr;
    struct IMFSample* input_sample_ = nullptr;
    struct IMFSample* output_sample_ = nullptr;
    struct IMFMediaBuffer* output_buffer_ = nullptr;
    DWORD steam_id_ = 0;

    bool previous_not_accepted_ = false;
};