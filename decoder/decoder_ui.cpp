#include "decoder_ui.hpp"
#include "decoder.hpp"
#include "logger.hpp"
#include <cstdio>
#include <cstdint>
#include <cstdlib>

#if defined(_WIN32)
#   include "decoder_windows.hpp"
#   include <d3d11.h>
#elif defined(__ANDROID__)
#   include "decoder_android.hpp"
#   include <android/log.h>
#elif defined(__APPLE__)
#   include "decoder_apple.hpp"
#endif


//------------------------------------------------------------------------------
DecoderUI::DecoderUI()
{
    logger_ = new Logger();
    decoder_ = new Decoder(*logger_);
}
//------------------------------------------------------------------------------
#if defined(_WIN32)
DecoderUI::DecoderUI(ID3D11Device* d3d_device, ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
{
    logger_ = new Logger();
    decoder_ = new DecoderWindows(*logger_, d3d_device, d3d_context, d3d_swapchain);
}
#endif
//------------------------------------------------------------------------------
DecoderUI::~DecoderUI()
{
    delete decoder_;
    delete logger_;
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

        if (ImGui::Button("Decoder", ImVec2(-1, 40))) { curr_page_ = 0; }
        ImGui::Spacing();
        if (ImGui::Button("Texture", ImVec2(-1, 40))) { curr_page_ = 1; }

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
            DrawPage0();
        else if (curr_page_ == 1)
            DrawPage1();
    }
    ImGui::EndChild();

    ImGui::End();
}
//------------------------------------------------------------------------------
void DecoderUI::DrawPage0()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Decoder");
    ImGui::Spacing();

    // Read media
    ImGui::BeginChild("MediaInfo", ImVec2(455, 400), true);
    {
        ImGui::InputTextMultiline(
            "##MediaInfo",
            media_text_,
            sizeof(media_text_),
            ImVec2(440, 300),
            ImGuiInputTextFlags_ReadOnly
        );        
        ImGui::Spacing();

        // Media path
        ImGui::InputText("MediaPath", media_path_, IM_ARRAYSIZE(media_path_));

        // Read button
        ImGui::PushStyleColor(ImGuiCol_Button, kGreen);
        if (ImGui::Button("1. Read", ImVec2(160, 40)))
        {
            memset(media_text_, 0, sizeof(media_text_));
            logger_->AddOutput(media_text_, sizeof(media_text_));
            decoder_->ReadMedia(media_path_);
            logger_->RemoveOutput(media_text_);
        }
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Create", ImVec2(455, 400), false);
    {
        // Create Section
        ImGui::BeginChild("Create", ImVec2(-1, 130), true);
        {
            ImGui::InputTextMultiline(
                "##Create",
                create_text_,
                sizeof(create_text_),
                ImVec2(-1, 70),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PushStyleColor(ImGuiCol_Button, kBlue);
            if (ImGui::Button("2. Create", ImVec2(160, 40)))
            {
                memset(create_text_, 0, sizeof(create_text_));
                logger_->AddOutput(create_text_, sizeof(create_text_));
                decoder_->CreateCodec();
                logger_->RemoveOutput(create_text_);
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        // Decode Section
        ImGui::BeginChild("Decode", ImVec2(-1, 130), true);
        {
            ImGui::InputTextMultiline(
                "##Decode",
                decode_text_,
                sizeof(decode_text_),
                ImVec2(-1, 70),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PushStyleColor(ImGuiCol_Button, kViolet);
            if (ImGui::Button("3. Decode", ImVec2(160, 40)))
            {
                memset(decode_text_, 0, sizeof(decode_text_));
                logger_->AddOutput(decode_text_, sizeof(decode_text_));
                decoder_->DecodeFrame();
                logger_->RemoveOutput(decode_text_);
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::BeginChild("Render", ImVec2(-1, 130), true);
        {
            ImGui::InputTextMultiline(
                "##Render",
                render_text_,
                sizeof(render_text_),
                ImVec2(-1, 70),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PushStyleColor(ImGuiCol_Button, kRed);
            if (ImGui::Button("4. Render", ImVec2(160, 40)))
            {
                memset(render_text_, 0, sizeof(render_text_));
                logger_->AddOutput(render_text_, sizeof(render_text_));
                decoder_->RenderFrame();
                logger_->RemoveOutput(render_text_);
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();

}
//------------------------------------------------------------------------------
void DecoderUI::DrawPage1()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Texture");
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