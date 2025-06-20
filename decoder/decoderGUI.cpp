#include "decoderGUI.hpp"
#include "decoder.hpp"
#include "imgui.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#if defined(__ANDROID__)
#   include <android/log.h>
#endif

//------------------------------------------------------------------------------
DecoderGUI::DecoderGUI(android_app* app)
{
    app_ = app;
    decoder_ = new Decoder(app, &DecoderGUI::Log);
}
//------------------------------------------------------------------------------
DecoderGUI::~DecoderGUI()
{
    delete decoder_;
}
//------------------------------------------------------------------------------
void DecoderGUI::DrawUI()
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
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
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
void DecoderGUI::DrawReadUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Read");
    ImGui::Spacing();

    // 檔案列表區塊
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f,0.13f,0.13f,1.0f));
    ImGui::BeginChild("FileList", ImVec2(0, 120), true, ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::TextColored(ImVec4(1,1,0.6f,1),"FileList");
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // 輸出路徑
    ImGui::InputText("OutPath", output_path_, IM_ARRAYSIZE(output_path_));
    // 檔名前綴
    ImGui::InputText("Prefix", prefix_, IM_ARRAYSIZE(prefix_));

    // 導出格式
    ImGui::Text("Format:");
    ImGui::RadioButton("Mp4", &export_format_, 0); ImGui::SameLine();
    ImGui::RadioButton("M4e", &export_format_, 1); ImGui::SameLine();
    ImGui::RadioButton("Mp4 & M4e", &export_format_, 2);
    ImGui::Spacing();

    // 全部重設按鈕（紅色）
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f,0.1f,0.1f,1.0f));
    if (ImGui::Button("Reset", ImVec2(120, 40)))
    {
        /*...*/
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // 開始按鈕（綠色）
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.5f,0.1f,1.0f));
    if (ImGui::Button("Start", ImVec2(120, 40)))
    {
        /*...*/
    }
    ImGui::PopStyleColor();
}
//------------------------------------------------------------------------------
void DecoderGUI::DrawDecodeUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Decode");
    ImGui::Spacing();
}
//------------------------------------------------------------------------------
void DecoderGUI::DrawRenderUI()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Render");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f,0.1f,0.1f,1.0f));
    if (ImGui::Button("CreateTexture", ImVec2(240, 40)))
    {
        int width = decoder_->width;
        int height = decoder_->height;
        uint8_t* data = (uint8_t*)malloc(width * height * 4);
        for (int i = 0; i < width * height; ++i)
        {
            data[i * 4 + 0] = 255; // R
            data[i * 4 + 1] = 255;   // G
            data[i * 4 + 2] = 0;   // B
            data[i * 4 + 3] = 255; // A
        }
        decoder_->textureID = Decoder::CreateTexture(width, height, data);
        free(data);
    }
    ImGui::PopStyleColor();

    if (decoder_->textureID != 0)
    {
        ImGui::Text("OpenGL Texture:");
        ImGui::Text("pointer = %x", decoder_->textureID);
        ImGui::Text("size = %d x %d", decoder_->width, decoder_->height);
        ImGui::Image((ImTextureID)(intptr_t)decoder_->textureID,
                     ImVec2((float)decoder_->width, (float)decoder_->height));
    }
}
//------------------------------------------------------------------------------
void DecoderGUI::Log(const char *fmt, ...)
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