//
//  Untitled.hpp
//  turn_clientB
//
//  Created by scarlett on 2025/6/24.
//

// Media Foundation + D3D11 解碼器與 Unity 整合
// 假設你已有串流資料接收器 (佇列部分)

#include <windows.h>
#include <mferror.h>
#include <mfapi.h>
#include <mftransform.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid.lib")

using namespace Microsoft::WRL;

// 全域變數
ComPtr<ID3D11Device> g_d3dDevice;
ComPtr<ID3D11DeviceContext> g_d3dContext;
ComPtr<IMFTransform> g_decoder;
ComPtr<ID3D11Texture2D> g_sharedTex;
HANDLE g_sharedHandle = nullptr;

int g_width = 1280;
int g_height = 720;

// 模擬資料佇列（請替換為實際接收程式），收到的壓縮H264 NALU資料暫存區
std::queue<std::vector<uint8_t>> g_h264Queue;
std::mutex g_mtxQueue;
std::condition_variable g_cvQueue;

bool PopH264(std::vector<uint8_t>& out) {
    std::unique_lock<std::mutex> lk(g_mtxQueue);
    g_cvQueue.wait(lk, []{ return !g_h264Queue.empty(); });
    out = std::move(g_h264Queue.front());
    g_h264Queue.pop();
    return true;
}

// 初始化 D3D11 裝置與共享紋理(ID3D11Texture2D)
bool InitD3D11DeviceAndSharedTexture() {
    // 建立裝置
    D3D_FEATURE_LEVEL featureLevel;
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                      D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
                      D3D11_SDK_VERSION, &g_d3dDevice, &featureLevel, &g_d3dContext);

    // 建立共享紋理
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = g_width;
    desc.Height = g_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    g_d3dDevice->CreateTexture2D(&desc, nullptr, &g_sharedTex);

    // 取得共享 Handle(給Unity用)
    ComPtr<IDXGIResource> dxgiRes;
    g_sharedTex.As(&dxgiRes);
    dxgiRes->GetSharedHandle(&g_sharedHandle);

    return true;
}

// 初始化 Media Foundation 解碼器，設定輸入/輸出格式
bool InitMFDecoder() {
    MFStartup(MF_VERSION);

    IMFActivate** activates = nullptr;
    UINT32 count = 0;
    MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, MFT_ENUM_FLAG_HARDWARE, nullptr, nullptr, &activates, &count);
    if (count == 0) return false;
    activates[0]->ActivateObject(IID_PPV_ARGS(&g_decoder));

    for (UINT32 i = 0; i < count; i++) activates[i]->Release();
    CoTaskMemFree(activates);

    // 輸入格式: H264
    ComPtr<IMFMediaType> inputType;
    MFCreateMediaType(&inputType);
    inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    inputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    g_decoder->SetInputType(0, inputType.Get(), 0);

    // 輸出格式: NV12 (可直接為 GPU 表面)
    ComPtr<IMFMediaType> outputType;
    MFCreateMediaType(&outputType);
    outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    MFSetAttributeSize(outputType.Get(), MF_MT_FRAME_SIZE, g_width, g_height);

    g_decoder->SetOutputType(0, outputType.Get(), 0);

    return true;
}

// 把串流壓縮資料塞進解碼器的Processinput()
void DecodeInputThread() {
    std::vector<uint8_t> packet;
    while (PopH264(packet)) {
        ComPtr<IMFMediaBuffer> buffer;
        MFCreateMemoryBuffer((DWORD)packet.size(), &buffer);
        BYTE* dst = nullptr;
        DWORD maxLen = 0;
        buffer->Lock(&dst, &maxLen, nullptr);
        memcpy(dst, packet.data(), packet.size());
        buffer->Unlock();
        buffer->SetCurrentLength((DWORD)packet.size());

        ComPtr<IMFSample> sample;
        MFCreateSample(&sample);
        sample->AddBuffer(buffer.Get());

        g_decoder->ProcessInput(0, sample.Get(), 0);
    }
}

// 把解碼結果從解碼器讀出，並直接寫入GPU texture
void DecodeOutputThread() {
    while (true) {
        MFT_OUTPUT_DATA_BUFFER output = {};
        DWORD status = 0;

        ComPtr<IMFSample> outSample;
        MFCreateSample(&outSample);

        // GPU Texture Buffer
        // MFCreateDXGISurfaceBuffer()是Media Foundation 提供的方式，可以用GPU texture做輸出
        ComPtr<IMFMediaBuffer> texBuffer;
        MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), g_sharedTex.Get(), 0, FALSE, &texBuffer);
        outSample->AddBuffer(texBuffer.Get());
        output.pSample = outSample.Get();

        HRESULT hr = g_decoder->ProcessOutput(0, 1, &output, &status);
        if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
            continue;

        // 若成功，資料已經寫入 GPU 紋理 g_sharedTex
    }
}

// 傳給 Unity
//GetSharedTextureHandle()將共享texture的HANDLE提供給Unity使用ExternalTexture
extern "C" __declspec(dllexport) HANDLE GetSharedTextureHandle() {
    return g_sharedHandle;
}
