#include "decoder_windows.hpp"
#include <cstdio>

#include <d3d11.h>

#include <mfapi.h>
#include <mferror.h>
#include <mftransform.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wmcodecdsp.h>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")

//------------------------------------------------------------------------------
DecoderWindows::DecoderWindows(LogFunc ui_logger, ID3D11Device* d3d_device, ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
    : Decoder(ui_logger)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

    d3d_device_ = d3d_device;
    d3d_context_ = d3d_context;
    d3d_swapchain_ = d3d_swapchain;
}
//------------------------------------------------------------------------------
DecoderWindows::~DecoderWindows()
{
    DestroyTexture();
    DestroyCodec();
    CoUninitialize();
    MFShutdown();
}
//------------------------------------------------------------------------------
bool DecoderWindows::ReadMedia(const char* file_path, char* media_info, size_t info_size)
{
    FILE* file = fopen(file_path, "r");
    if (file == nullptr)
    {
        snprintf(media_info, info_size, "Failed to open file: %s", file_path);
        return false;
    }

    for (size_t i = 0; i < (info_size / 3) - 1; i += 3)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            media_info[i] = '\0';
            break;
        }
        snprintf(media_info + i, info_size, "%02X ", c);
    }

    fclose(file);
    return true;
}
//------------------------------------------------------------------------------
bool DecoderWindows::CreateTexture(const void* data)
{
    DestroyTexture();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = d3d_device_->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(hr))
    {
        Log("CreateTexture: Failed to create texture, HRESULT: %lx", hr);
        return false;
    }

    if (data)
        d3d_context_->UpdateSubresource(texture, 0, nullptr, data, width * 4, 0);

    // create ShaderResourceView
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = d3d_device_->CreateShaderResourceView(texture, &srvDesc, &srv);
    texture->Release(); // SRV will AddRef

    if (FAILED(hr))
    {
        Log("CreateTexture: Failed to create SRV, HRESULT: %lx", hr);
        return false;
    }

    textureID = reinterpret_cast<intptr_t>(srv);
    return true;
}
//------------------------------------------------------------------------------
void DecoderWindows::DestroyTexture()
{
    if (textureID != 0)
    {
        ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(textureID);
        srv->Release();
        textureID = 0;
    }
}
//------------------------------------------------------------------------------
bool DecoderWindows::CreateCodec()
{
    Log("CreateCodec: Not implemented yet.");
    return true;
}
//------------------------------------------------------------------------------
void DecoderWindows::DestroyCodec()
{
    Log("DestroyCodec: Not implemented yet.");
}