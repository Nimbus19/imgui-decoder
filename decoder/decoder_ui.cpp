#include "decoder_ui.hpp"
#include "decoder.hpp"
#include "imgui.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#if defined(_WIN32)
#   include "decoder_windows.hpp"
#elif defined(__ANDROID__)
#   include "decoder_android.hpp"
#   include <android/log.h>
#elif defined(__APPLE__)
#   include "decoder_apple.hpp"
#endif


//------------------------------------------------------------------------------
DecoderUI::DecoderUI()
{

}
//------------------------------------------------------------------------------
#if defined(_WIN32)
DecoderUI::DecoderUI(ID3D11Device* d3d_device, ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
{
    decoder_ = new DecoderWindows(&DecoderUI::Log, d3d_device, d3d_context, d3d_swapchain);
}
#endif
//------------------------------------------------------------------------------
DecoderUI::~DecoderUI()
{
    delete decoder_;
}
//------------------------------------------------------------------------------
void DecoderUI::DrawUI()
{
    // 設定視窗大小
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    // 左側功能選單
    ImGui::BeginChild("Sidebar", ImVec2(220, 0), true, ImGuiWindowFlags_NoScrollbar);
    {

        if (ImGui::Button("Read", ImVec2(-1, 40))) { curr_page_ = 0; }
        ImGui::Spacing();
        if (ImGui::Button("Decode", ImVec2(-1, 40))) { curr_page_ = 1; }
        ImGui::Spacing();
        if (ImGui::Button("Render", ImVec2(-1, 40))) { curr_page_ = 2; }

        // 版本顯示
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 20);
        ImGui::Text("%s", kVersion);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 右側主作業區
    ImGui::BeginChild("Page", ImVec2(0, 0), true);
    {
        if (curr_page_ == 0)
            DrawReadUI();
        else if (curr_page_ == 1)
            DrawDecodeUI();
        else if (curr_page_ == 2)
            DrawRenderUI();
    }
    ImGui::EndChild();

    ImGui::End();
}
//------------------------------------------------------------------------------
void DecoderUI::DrawReadUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Read");
    ImGui::Spacing();

    // Media Info
    ImGui::BeginChild("MediaInfo", ImVec2(0, 320), true, ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::InputTextMultiline(
        "##MediaInfo",
        file_buffer_,
        sizeof(file_buffer_),
        ImVec2(-1, -1),
        ImGuiInputTextFlags_ReadOnly // selectable but readonly
    );
    ImGui::EndChild();
    ImGui::Spacing();

    // Media path
    ImGui::InputText("MediaPath", media_path_, IM_ARRAYSIZE(media_path_));

    // Reset button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f,0.1f,0.1f,1.0f));
    if (ImGui::Button("Reset", ImVec2(120, 40)))
    {
        memset(file_buffer_, 0, sizeof(file_buffer_));
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // Read button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.5f,0.1f,1.0f));
    if (ImGui::Button("Read", ImVec2(120, 40)))
    {
        decoder_->ReadMedia(media_path_, file_buffer_, sizeof(file_buffer_));
    }
    ImGui::PopStyleColor();
}
//------------------------------------------------------------------------------
void DecoderUI::DrawDecodeUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Decode");
    ImGui::Spacing();
}
//------------------------------------------------------------------------------
void DecoderUI::DrawRenderUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Render");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f,0.1f,0.1f,1.0f));
    if (ImGui::Button("CreateTexture", ImVec2(240, 40)))
    {
        int width = decoder_->width;
        int height = decoder_->height;
        uint8_t* data = (uint8_t*)malloc(width * height * 4);
        if (data)
        {
            for (int i = 0; i < width * height; ++i)
            {
                data[i * 4 + 0] = 255; // R
                data[i * 4 + 1] = 255;   // G
                data[i * 4 + 2] = 0;   // B
                data[i * 4 + 3] = 255; // A
            }
            decoder_->CreateTexture(data);
            free(data);
        }
    }
    ImGui::PopStyleColor();

    if (decoder_->textureID != 0)
    {
        ImGui::Text("Texture:");
        ImGui::Text("pointer = %x", decoder_->textureID);
        ImGui::Text("size = %d x %d", decoder_->width, decoder_->height);
        ImGui::Image((ImTextureID)(intptr_t)decoder_->textureID,
                     ImVec2((float)decoder_->width, (float)decoder_->height));
    }
}
//------------------------------------------------------------------------------
void DecoderUI::Log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if defined(_WIN32)

#elif defined(__ANDROID__)
    __android_log_vprint(ANDROID_LOG_INFO, "Decoder", fmt, args);
#elif defined(__APPLE__)

#endif
    va_end(args);
}