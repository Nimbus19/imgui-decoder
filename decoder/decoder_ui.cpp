#include "decoder_ui.hpp"
#include "decoder.hpp"
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
    log_ = ConsoleLog;
}
//------------------------------------------------------------------------------
#if defined(_WIN32)
DecoderUI::DecoderUI(ID3D11Device* d3d_device, ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
{
    log_ = ConsoleLog;
    decoder_ = new DecoderWindows(log_, d3d_device, d3d_context, d3d_swapchain);
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
        media_text_,
        sizeof(media_text_),
        ImVec2(-1, -1),
        ImGuiInputTextFlags_ReadOnly // selectable but readonly
    );
    ImGui::EndChild();
    ImGui::Spacing();

    // Media path
    ImGui::InputText("MediaPath", media_path_, IM_ARRAYSIZE(media_path_));

    // Read button
    ImGui::PushStyleColor(ImGuiCol_Button, kGreen);
    if (ImGui::Button("Read", ImVec2(120, 40)))
    {
        decoder_->ReadMedia(media_path_, media_text_, sizeof(media_text_));
    }
    ImGui::PopStyleColor();
}
//------------------------------------------------------------------------------
void DecoderUI::DrawDecodeUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Decode");
    ImGui::Spacing();

    // Text Section
    ImGui::BeginChild("Create", ImVec2(160, 120), true);
    ImGui::InputTextMultiline(
        "##Create",
        create_text_,
        sizeof(create_text_),
        ImVec2(-1, -1),
        ImGuiInputTextFlags_ReadOnly
    );
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("Decode", ImVec2(160, 120), true);
    ImGui::InputTextMultiline(
        "##Decode",
        decode_text_,
        sizeof(decode_text_),
        ImVec2(-1, -1),
        ImGuiInputTextFlags_ReadOnly
    );
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("Render", ImVec2(160, 120), true);
    ImGui::InputTextMultiline(
        "##Render",
        render_text_,
        sizeof(render_text_),
        ImVec2(-1, -1),
        ImGuiInputTextFlags_ReadOnly
    );
    ImGui::EndChild();
    ImGui::Spacing();

    // Create button
    ImGui::PushStyleColor(ImGuiCol_Button, kRed);
    if (ImGui::Button("Create", ImVec2(160, 40)))
    {
        decoder_->CreateCodec();
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();

    // Decode button
    if (ImGui::Button("Decode", ImVec2(160, 40)))
    {
        decoder_->DecodeFrame();
    }
    ImGui::SameLine();

    // Render button
    ImGui::PushStyleColor(ImGuiCol_Button, kGreen);
    if (ImGui::Button("Render", ImVec2(160, 40)))
    {
        decoder_->RenderFrame();
    }
    ImGui::PopStyleColor();

    // Show texture
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
void DecoderUI::ConsoleLog(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if defined(_WIN32)
    char temp[4096];
    vsnprintf(temp, 4096, fmt, args);
    OutputDebugStringA(temp);
#elif defined(__ANDROID__)
    __android_log_vprint(ANDROID_LOG_INFO, "Decoder", fmt, args);
#elif defined(__APPLE__)

#endif
    va_end(args);
}