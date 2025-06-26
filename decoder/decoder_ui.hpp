#pragma once

#include "imgui.h"
#include <cstdint>
#include <d3d11.h>

class DecoderUI
{
public:
    using LogFunc = void(*)(const char*, ...);
    const char kVersion[8] = "v 0.0.0";

    explicit DecoderUI();
#if defined(_WIN32)
    explicit DecoderUI(ID3D11Device* d3d_device, ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain);
#elif defined(__ANDROID__)
    explicit DecoderUI();
#elif defined(__APPLE__)
    explicit DecoderUI();
#endif
    virtual ~DecoderUI();

    void DrawUI();
    void DrawReadUI();
    void DrawDecodeUI();

protected:
    const ImVec4 kGreen = ImVec4(0.1f, 0.5f, 0.1f, 1.0f);
    const ImVec4 kRed = ImVec4(0.5f, 0.1f, 0.1f, 1.0f);

    static void ConsoleLog(const char* fmt, ...);

    LogFunc log_ = nullptr;    
    class Decoder* decoder_ = nullptr;
    int curr_page_ = 0; // 0: Read, 1: Decode
#if defined(_WIN32)
    char media_path_[256] = "E:\\Videos\\apple_bipbop\\bipbop.mp4";
#elif defined(__ANDROID__)
    char media_path_[256] = "/sdcard/Download/apple_bipbop/bipbop.mp4";
#elif defined(__APPLE__)
    char media_path_[256] = "apple_bipbop/bipbop.mp4";
#else
    char media_path_[256] = "apple_bipbop/bipbop.mp4";
#endif
    char media_text_[2048] = "Media Info";
    char create_text_[512] = "Create Info";
    char decode_text_[512] = "Decode Info";
    char render_text_[512] = "Render Info";
};