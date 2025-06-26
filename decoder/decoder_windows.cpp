#include "decoder_windows.hpp"
#include "decoder_windows_guid.hpp"
#include <cstdio>
#include <vector>

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
template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
//------------------------------------------------------------------------------
DecoderWindows::DecoderWindows(LogFunc ui_logger, ID3D11Device* d3d_device, 
    ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
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
    SafeRelease(&reader_);
    SafeRelease(&input_type_);

    DestroyTexture();
    DestroyCodec();

    CoUninitialize();
    MFShutdown();
}
//------------------------------------------------------------------------------
bool DecoderWindows::ReadMedia(const char* file_path, char* media_info, size_t info_size)
{
    SafeRelease(&reader_);
    SafeRelease(&input_type_);

    HRESULT hr = S_OK;    

    int wstr_size = MultiByteToWideChar(CP_UTF8, 0, file_path, -1, nullptr, 0);
    std::vector<wchar_t> wstr_path(wstr_size);
    MultiByteToWideChar(CP_UTF8, 0, file_path, -1, wstr_path.data(), wstr_size);

    hr = MFCreateSourceReaderFromURL(wstr_path.data(), nullptr, &reader_);
    if (FAILED(hr))
        return false;

    hr = reader_->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
    if (FAILED(hr))
        return false;

    hr = reader_->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &input_type_);
    if (FAILED(hr))
        return false;

    PrintMediaType(input_type_, media_info, info_size);

    return true;
}
//------------------------------------------------------------------------------
void DecoderWindows::PrintMediaType(IMFMediaType* type, char* buffer, size_t buffer_size)
{
    if (!type || !buffer || buffer_size == 0)
        return;
    buffer[0] = '\0';

    UINT32 count = 0;
    if (FAILED(type->GetCount(&count)))
        return;

    for (UINT32 i = 0; i < count; ++i)
    {
        GUID guid = {};
        PROPVARIANT var;
        PropVariantInit(&var);

        if (SUCCEEDED(type->GetItemByIndex(i, &guid, &var)))
        {
            const char* guidName = GuidToName(guid);
            LPOLESTR guidStr = nullptr;
            HRESULT hr = StringFromCLSID(guid, &guidStr);

            char line[256] = {};
            const char* nameToShow = guidName ? guidName : "";

            if (var.vt == VT_UI4)
                snprintf(line, sizeof(line), "%s%ws: %u\n", nameToShow, guidName ? L"" : guidStr, var.ulVal);
            else if (var.vt == VT_UI8)
                snprintf(line, sizeof(line), "%s%ws: %llu\n", nameToShow, guidName ? L"" : guidStr, var.uhVal.QuadPart);
            else if (var.vt == VT_R8)
                snprintf(line, sizeof(line), "%s%ws: %f\n", nameToShow, guidName ? L"" : guidStr, var.dblVal);
            else if (var.vt == VT_CLSID && var.puuid)
            {
                const char* valuName = GuidToName(*var.puuid);
                const char* valuToShow = valuName ? valuName : "";
                LPOLESTR valueStr = nullptr;
                StringFromCLSID(*var.puuid, &valueStr);
                snprintf(line, sizeof(line), "%s%ws: %s%ws\n", nameToShow, guidName ? L"" : guidStr, valuToShow, valuName ? L"" : valueStr);
                if (valueStr) CoTaskMemFree(valueStr);
            }
            else if (var.vt == VT_LPWSTR)
                snprintf(line, sizeof(line), "%s%ws: %ws\n", nameToShow, guidName ? L"" : guidStr, var.pwszVal);
            else
                snprintf(line, sizeof(line), "%s%ws: [type %d]\n", nameToShow, guidName ? L"" : guidStr, var.vt);

            size_t curr_len = strlen(buffer);
            size_t remain = buffer_size > curr_len ? buffer_size - curr_len : 0;
            if (remain > 1)
                strncat(buffer, line, remain - 1);

            if (guidStr) CoTaskMemFree(guidStr);
            PropVariantClear(&var);
        }
    }
}
//------------------------------------------------------------------------------
const char* DecoderWindows::GuidToName(const GUID& guid) 
{
    for (const auto& entry : g_guidNameMap) 
    {
        if (IsEqualGUID(guid, *entry.guid))
            return entry.name;
    }
    return nullptr;
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
    SafeRelease(&codec_);

    HRESULT hr = S_OK;
    hr = CoCreateInstance(CLSID_CMSAACDecMFT, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&codec_));
    if (FAILED(hr))
        return false;

    hr = codec_->SetInputType(0, input_type_, 0);
    if (FAILED(hr))
        return false;

    codec_->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
    codec_->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
    codec_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);

    return true;
}
//------------------------------------------------------------------------------
void DecoderWindows::DestroyCodec()
{
    SafeRelease(&codec_);
    SafeRelease(&output_type_);    
    SafeRelease(&output_sample_);
    SafeRelease(&output_buffer_);
}
