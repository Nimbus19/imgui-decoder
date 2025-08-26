#include "decoder_windows.hpp"

#include <cstdio>
#include <vector>
#include <string>

#include <d3d11.h>

#include <mfapi.h>
#include <mferror.h>
#include <mftransform.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wmcodecdsp.h>

#include "logger.hpp"
#include "decoder_windows_guid.hpp"

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")

#define CODEC_DIRECT_OUTPUT 1 // Enable decoder direct output to texture
#define CODEC_OUTPUT_FORMAT MFVideoFormat_NV12 // Each 3 bytes represent 2 pixels
#define PIXEL_WIDTH 1.5f // NV12 format has 1.5 bytes per pixel
#define TEXTURE_FORMAT DXGI_FORMAT_R8_UNORM

//------------------------------------------------------------------------------
DecoderWindows::DecoderWindows(Logger* logger, ID3D11Device* d3d_device,
    ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
    : Decoder(logger)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        PrintWinError("CoInitializeEx", hr);
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

    DestroySRV();
    DestroyTexture();
    DestroyCodec();

    CoUninitialize();
    MFShutdown();
}
//------------------------------------------------------------------------------
bool DecoderWindows::ReadMedia(const char* file_path)
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

    hr = reader_->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
    if (FAILED(hr))
        return false;

    hr = reader_->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &input_type_);
    if (FAILED(hr))
        return false;

    PrintMediaType(input_type_);

    return true;
}
//------------------------------------------------------------------------------
void DecoderWindows::PrintMediaType(IMFMediaType* type)
{
    if (!type)
        return;

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
            const char* guid_name = GuidToName(guid);
            const char* name_to_show = guid_name ? guid_name : "";

            LPOLESTR guid_str = nullptr;
            HRESULT hr = StringFromCLSID(guid, &guid_str);
            const wchar_t* guid_to_show = !guid_name && SUCCEEDED(hr) ?guid_str : L"";

            if (var.vt == VT_UI4)
                Log("%s%ws: %u\n", name_to_show, guid_to_show, var.ulVal);
            else if (var.vt == VT_UI8)
                Log("%s%ws: %llu\n", name_to_show, guid_to_show, var.uhVal.QuadPart);
            else if (var.vt == VT_R8)
                Log("%s%ws: %f\n", name_to_show, guid_to_show, var.dblVal);
            else if (var.vt == VT_CLSID && var.puuid)
            {
                const char* value_name = GuidToName(*var.puuid);
                const char* value_to_show = value_name ? value_name : "";

                LPOLESTR value_guid_str = nullptr;
                hr = StringFromCLSID(*var.puuid, &value_guid_str);
                const wchar_t* value_guid_to_show = !value_name && SUCCEEDED(hr) ? value_guid_str : L"";

                Log("%s%ws: %s%ws\n", name_to_show, guid_to_show, value_to_show, value_guid_to_show);
                if (value_guid_str)
                    CoTaskMemFree(value_guid_str);
            }
            else if (var.vt == VT_LPWSTR)
                Log("%s%ws: %ws\n", name_to_show, guid_to_show, var.pwszVal);
            else
                Log("%s%ws: [type %d]\n", name_to_show, guid_to_show, var.vt);

            if (guid_str) 
                CoTaskMemFree(guid_str);

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
bool DecoderWindows::CreateTexture()
{
    DestroyTexture();

    texture_width = video_width;
    texture_height = (int)(video_height * PIXEL_WIDTH);

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = texture_width;
    desc.Height = texture_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = TEXTURE_FORMAT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    HRESULT hr = d3d_device_->CreateTexture2D(&desc, nullptr, &d3d_texture_);
    if (FAILED(hr))
    {
        PrintWinError("CreateTexture2D", hr);
        return false;
    }
    textureID = reinterpret_cast<intptr_t>(d3d_texture_);
    return true;
}
//------------------------------------------------------------------------------
bool DecoderWindows::UpdateTexture(const void* data)
{
    if (d3d_context_ && data)
        d3d_context_->UpdateSubresource(d3d_texture_, 0, nullptr, data, texture_width, 0);
    return true;
}
//------------------------------------------------------------------------------
void DecoderWindows::DestroyTexture()
{
    SafeRelease(&d3d_texture_);
    d3d_texture_ = nullptr;
    textureID = 0;
}
//------------------------------------------------------------------------------
ID3D11ShaderResourceView* DecoderWindows::CreateSRV()
{
    // create ShaderResourceView
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = TEXTURE_FORMAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    HRESULT hr = d3d_device_->CreateShaderResourceView(d3d_texture_, &srvDesc, &srv);
    if (FAILED(hr))
    {
        PrintWinError("CreateShaderResourceView", hr);
        return nullptr;
    }

    Log("ShaderResourceView created successfully\n");
    return srv;
}
//------------------------------------------------------------------------------
void DecoderWindows::DestroySRV()
{
    SafeRelease(&srv);
    srv = nullptr;
}
//------------------------------------------------------------------------------
bool DecoderWindows::CreateCodec()
{
    if (!input_type_)
        return false;

    DestroyCodec();

    HRESULT hr = S_OK;
    hr = CoCreateInstance(CLSID_CMSH264DecoderMFT, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&codec_));
    if (FAILED(hr))
        return false;

    hr = codec_->SetInputType(0, input_type_, 0);
    if (FAILED(hr))
        return false;

    codec_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
    codec_->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
    codec_->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
    Log("Codec created successfully\n");

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
//------------------------------------------------------------------------------
bool DecoderWindows::DecodeFrame()
{
    if (!reader_ || !codec_)
        return false;

    HRESULT hr = E_FAIL;
    DWORD flags;
    LONGLONG timestamp;

    if (!previous_not_accepted_)
    {
        hr = reader_->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            &steam_id_, &flags, &timestamp, &input_sample_);
    }

    if (input_sample_)
    {
        hr = codec_->ProcessInput(0, input_sample_, 0);
        switch (hr)
        {
        case S_OK:
            Log("ProcessInput : S_OK\n");
            previous_not_accepted_ = false;
            break;
        case MF_E_NOTACCEPTING:
            Log("ProcessInput : MF_E_NOTACCEPTING\n");
            previous_not_accepted_ = true;
            return false;
        default:
            PrintWinError("ProcessInput", hr);
            break;
        }

        LONGLONG pts;
        input_sample_->GetSampleTime(&pts);
        Log("PTS : %lld\n", pts);

        DWORD total_length = 0;
        input_sample_->GetTotalLength(&total_length);
        Log("Size : %d\n", total_length);
    }

    SafeRelease(&input_sample_);
    return SUCCEEDED(hr);
}
//------------------------------------------------------------------------------
bool DecoderWindows::RenderFrame()
{
    if (!codec_)
        return false;

    HRESULT hr = E_FAIL;
    DWORD output_status = 0;
    MFT_OUTPUT_DATA_BUFFER output_buffer = { 0, output_sample_, 0, NULL };

    hr = codec_->ProcessOutput(MFT_OUTPUT_STREAM_LAZY_READ, 1, &output_buffer, &output_status);

    switch (hr)
    {
    case S_OK:
    {
        Log("ProcessOutput : S_OK\n");

        IMFSample* output_sample = output_buffer.pSample;

        LONGLONG sample_time;
        hr = output_sample->GetSampleTime(&sample_time);
        Log("%s : %lld\n", "PTS", sample_time);

#if !CODEC_DIRECT_OUTPUT
        DWORD output_count = 0;
        hr = output_sample->GetBufferCount(&output_count);

        for (DWORD buf_index = 0; buf_index < output_count; buf_index++)
        {
            IMFMediaBuffer* output_media_buffer;
            hr = output_sample->GetBufferByIndex(buf_index, &output_media_buffer);
            if (FAILED(hr))
                break;

            DWORD total_length = 0;
            hr = output_media_buffer->GetCurrentLength(&total_length);
            if (FAILED(hr))
                break;
            Log("%s : %d\n", "Size", total_length);

            BYTE* buffer_start;
            hr = output_media_buffer->Lock(&buffer_start, NULL, NULL);
            if (FAILED(hr))
                break;
            
            UpdateTexture((uint8_t*)buffer_start);

            hr = output_media_buffer->Unlock();
            if (FAILED(hr))
                break;

            hr = output_media_buffer->SetCurrentLength(0);
            if (FAILED(hr))
                break;
        }
#endif
        break;
    }
    case MF_E_TRANSFORM_TYPE_NOT_SET:
    case MF_E_TRANSFORM_STREAM_CHANGE:
    {
        Log("ProcessOutput : MF_E_TRANSFORM_STREAM_CHANGE\n");

        if (SetOutputType())
        {
            if (!CreateTexture())
            {
                PrintWinError("CreateTexture", hr);
                return false;
            }
            if (!AllocateOutputSample())
            {
                PrintWinError("AllocateOutputSample", hr);
                return false;
            }
        }
        PrintMediaType(output_type_);
        break;
    }
    case MF_E_TRANSFORM_NEED_MORE_INPUT:
    {
        Log("ProcessOutput : MF_E_TRANSFORM_NEED_MORE_INPUT\n");
        break;
    }
    default:
    {
        PrintWinError("ProcessOutput", hr);
        break;
    }
    }

    return SUCCEEDED(hr);
}
//------------------------------------------------------------------------------
bool DecoderWindows::SetOutputType()
{
    HRESULT hr = S_OK;

    MFCreateMediaType(&output_type_);
    output_type_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    output_type_->SetGUID(MF_MT_SUBTYPE, CODEC_OUTPUT_FORMAT);

    for (DWORD i = 0; i < 1024; ++i)
    {
        if (codec_->GetOutputAvailableType(0, i, &output_type_) != S_OK)
            break;
        GUID subtype = {};
        output_type_->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (subtype == CODEC_OUTPUT_FORMAT)
            break;
    }

    hr = codec_->SetOutputType(0, output_type_, 0);
    if (FAILED(hr))
    {
        PrintWinError("SetOutputType", hr);
        return false;
    }

    UINT32 w = 0, h = 0;
    MFGetAttributeSize(output_type_, MF_MT_FRAME_SIZE, &w, &h);

    if (video_width == w && video_height == h)
        return false;

    video_width = w;
    video_height = h;

    return true;
}
//------------------------------------------------------------------------------
bool DecoderWindows::AllocateOutputSample()
{
    HRESULT hr;
    SafeRelease(&output_buffer_);
    SafeRelease(&output_sample_);

    MFT_OUTPUT_STREAM_INFO output_info;

    hr = codec_->GetOutputStreamInfo(0, &output_info);
    if (FAILED(hr))
        return false;

    if (output_info.dwFlags & (MFT_OUTPUT_STREAM_PROVIDES_SAMPLES | MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES))
    {
        /* The MFT will provide an allocated sample. */
        return true;
    }

    hr = MFCreateSample(&output_sample_);
    if (FAILED(hr))
        return false;

#if CODEC_DIRECT_OUTPUT
    // Create a media buffer from texture
    hr = MFCreateDXGISurfaceBuffer(
        __uuidof(ID3D11Texture2D), d3d_texture_, 0, FALSE, &output_buffer_);
    if (FAILED(hr))
        return false;
#else
    DWORD allocation_size;
    DWORD alignment;
    allocation_size = output_info.cbSize;
    alignment = output_info.cbAlignment;
    if (alignment > 0)
        hr = MFCreateAlignedMemoryBuffer(allocation_size, alignment - 1, &output_buffer_);
    else
        hr = MFCreateMemoryBuffer(allocation_size, &output_buffer_);
#endif 

    hr = output_sample_->AddBuffer(output_buffer_);
    if (FAILED(hr))
        return false;

    return true;
}
//------------------------------------------------------------------------------
template <class T> void DecoderWindows::SafeRelease(T** ppT)
{
    if (*ppT)
    {
        ULONG ref_count = (*ppT)->Release();
        if (ref_count != 0)
        {
            Log("Object not released, ref_count = %lu\n", ref_count);
        }
        *ppT = NULL;
    }
}
//------------------------------------------------------------------------------
void DecoderWindows::PrintWinError(const char* funcName, HRESULT hr)
{
    // Get Windows Error
    LPSTR lpMsgBuf;
    DWORD bufLen = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPSTR)&lpMsgBuf,
        0,
        NULL);

    if (bufLen)
    {
        std::string errorMessage(lpMsgBuf, bufLen);
        Log("%s failed. \n  WinError: %s\n", funcName, errorMessage.c_str());
    }
    else
    {
        // Get MF Error
        auto it = g_mfErrorMap.find(hr);
        if (it != g_mfErrorMap.end())
            Log("%s failed. \n  MFError: %s\n", funcName, it->second);
        else
            Log("%s failed. \n  Error: 0x%08X\n", funcName, hr);
    }
    LocalFree(lpMsgBuf);
}
//------------------------------------------------------------------------------