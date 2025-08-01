#pragma once

#include "imgui.h"

class DecoderUI
{
public:
    const char kVersion[8] = "v 0.0.0";

    explicit DecoderUI();
#if defined(_WIN32)
    explicit DecoderUI(
        struct ID3D11Device* d3d_device, 
        struct ID3D11DeviceContext* d3d_context,
        struct IDXGISwapChain* d3d_swapchain );
#elif defined(__ANDROID__)
    explicit DecoderUI(
        struct android_app* g_app );
#elif defined(__APPLE__)
#endif
    virtual ~DecoderUI();

    void DrawUI();
    void DrawPage0();
    void DrawPage1();

protected:
    const ImVec4 kRed       = ImVec4(1.00f, 0.23f, 0.19f, 0.4f);
    const ImVec4 kGreen     = ImVec4(0.30f, 0.85f, 0.39f, 0.4f);
    const ImVec4 kBlue      = ImVec4(0.00f, 0.48f, 1.00f, 0.4f);
    const ImVec4 kViolet    = ImVec4(0.69f, 0.32f, 0.87f, 0.4f);

    void ShowTexture(float scale);

    class Logger* logger_;
    class Decoder* decoder_;

    int curr_page_ = 0; // 0: Decoder, 1: Texture
#if defined(_WIN32)
    char media_path_[256] = "E:\\Videos\\apple_bipbop\\bipbop.mp4";
#elif defined(__ANDROID__)
    char media_path_[256] = "/sdcard/Download/apple_bipbop/bipbop.mp4";
#elif defined(__APPLE__)
    char media_path_[256] = "apple_bipbop/bipbop.mp4";
#else
    char media_path_[256] = "apple_bipbop/bipbop.mp4";
#endif
    char media_text_[2048] = "Media Info\n";
    char create_text_[2048] = "Create Info\n";
    char decode_text_[2048] = "Decode Info\n";
    char render_text_[2048] = "Render Info\n";
};