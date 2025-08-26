#include "decoder_ui.hpp"

#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include "logger.hpp"
#include "decoder.hpp"

#if defined(_WIN32)
#   include <d3d11.h>
#   include "decoder_windows.hpp"
#   define USE_SQUARE_LAYOUT true
#   define SECTION_RATIO 1.0f
#   define BUTTON_RATIO 1.0f
#elif defined(__ANDROID__)
#   include <android/log.h>
#   include "imgui_impl_android.h"
#   include "imgui_impl_opengl3.h"
#   include "decoder_android.hpp"
#   define USE_SQUARE_LAYOUT false
#   define SECTION_RATIO 2.25f
#   define BUTTON_RATIO 1.5f
#elif defined(__APPLE__)
#   include "decoder_apple.hpp"
#   define USE_SQUARE_LAYOUT true
#   define SECTION_RATIO 1.0f
#   define BUTTON_RATIO 1.0f
#endif


//------------------------------------------------------------------------------
DecoderUI::DecoderUI()
{
    logger_ = new Logger();
    decoder_ = new Decoder(logger_);
}
//------------------------------------------------------------------------------
#if defined(_WIN32)
DecoderUI::DecoderUI(ID3D11Device* d3d_device, ID3D11DeviceContext* d3d_context, IDXGISwapChain* d3d_swapchain)
{
    logger_ = new Logger();
    decoder_ = new DecoderWindows(logger_, d3d_device, d3d_context, d3d_swapchain);
}
#elif defined(__ANDROID__)
DecoderUI::DecoderUI(android_app* g_app)
{
    logger_ = new Logger();
    decoder_ = new DecoderAndroid(logger_, g_app);
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

    // Left Section
    ImGui::BeginChild("MediaInfo", ImVec2(455, 400 * SECTION_RATIO), true);
    {
        // Media path
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##MediaPath", media_path_, IM_ARRAYSIZE(media_path_));
        ImGui::PopItemWidth();

        // Media info text area
        ImGui::InputTextMultiline(
            "##MediaInfo",
            media_text_,
            sizeof(media_text_),
            ImVec2(-1, 300 * SECTION_RATIO),
            ImGuiInputTextFlags_ReadOnly
        );        
        ImGui::Spacing();

        // Read button
        ImGui::PushStyleColor(ImGuiCol_Button, kGreen);
        if (ImGui::Button("1. Read", ImVec2(160, 40 * BUTTON_RATIO)))
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

    // Right Section
    ImGui::BeginChild("Decoder", ImVec2(455, 410 * SECTION_RATIO), false);
    {
        // Create Section
        ImGui::BeginChild("Create", ImVec2(-1, 130 * SECTION_RATIO), true);
        {
            ImGui::InputTextMultiline(
                "##Create",
                create_text_,
                sizeof(create_text_),
                ImVec2(-1, 70 * SECTION_RATIO),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PushStyleColor(ImGuiCol_Button, kBlue);
            if (ImGui::Button("2. Create", ImVec2(160, 40 * BUTTON_RATIO)))
            {
                memset(create_text_, 0, sizeof(create_text_));
                logger_->AddOutput(create_text_, sizeof(create_text_));
                if (decoder_->CreateCodec())
                {
                    decode_count_ = 0;
                    render_count_ = 0;
                }
                logger_->RemoveOutput(create_text_);

            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        // Decode Section
        ImGui::BeginChild("Decode", ImVec2(-1, 130 * SECTION_RATIO), true);
        {
            ImGui::InputTextMultiline(
                "##Decode",
                decode_text_,
                sizeof(decode_text_),
                ImVec2(-1, 70 * SECTION_RATIO),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PushStyleColor(ImGuiCol_Button, kViolet);
            if (ImGui::Button("3. Decode", ImVec2(160, 40 * BUTTON_RATIO)))
            {
                memset(decode_text_, 0, sizeof(decode_text_));
                logger_->AddOutput(decode_text_, sizeof(decode_text_));
                if (decoder_->DecodeFrame())
                    decode_count_++;
                logger_->RemoveOutput(decode_text_);
            }
            ImGui::SameLine();
            if (ImGui::Button("Decode x 10", ImVec2(160, 40 * BUTTON_RATIO)))
            {
                memset(decode_text_, 0, sizeof(decode_text_));
                logger_->AddOutput(decode_text_, sizeof(decode_text_));
                for (int i = 0; i < 10; ++i)
                {
                    if (!decoder_->DecodeFrame())
                        break;
                    decode_count_++;
                }
                logger_->RemoveOutput(decode_text_);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("%d", decode_count_);
        }
        ImGui::EndChild();

        // Render Section
        ImGui::BeginChild("Render", ImVec2(-1, 130 * SECTION_RATIO), true);
        {
            ImGui::InputTextMultiline(
                "##Render",
                render_text_,
                sizeof(render_text_),
                ImVec2(-1, 70 * SECTION_RATIO),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PushStyleColor(ImGuiCol_Button, kRed);
            if (ImGui::Button("4. Render", ImVec2(160, 40 * BUTTON_RATIO)))
            {
                memset(render_text_, 0, sizeof(render_text_));
                logger_->AddOutput(render_text_, sizeof(render_text_));
                if (decoder_->RenderFrame())
                    render_count_++;
                logger_->RemoveOutput(render_text_);
            }
            ImGui::SameLine();
            if (ImGui::Button("Render x 10", ImVec2(160, 40 * BUTTON_RATIO)))
            {
                memset(render_text_, 0, sizeof(render_text_));
                logger_->AddOutput(render_text_, sizeof(render_text_));
                for (int i = 0; i < 10; ++i)
                {
                    if (!decoder_->RenderFrame())
                        break;
                    render_count_++;
                }
                logger_->RemoveOutput(render_text_);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("%d", render_count_);
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();

#if USE_SQUARE_LAYOUT
    ImGui::Spacing();
#else
    ImGui::SameLine();
#endif

    // Show texture
    ImGui::BeginChild("Texture", ImVec2(917, -1), true);
    {
        ShowTexture(5 / BUTTON_RATIO / BUTTON_RATIO);
    }
    ImGui::EndChild();

}
//------------------------------------------------------------------------------
void DecoderUI::DrawPage1()
{
    ImGui::TextColored(ImVec4(1,1,1,1), "Texture");
    // Show texture
    ShowTexture(1.6 / BUTTON_RATIO / BUTTON_RATIO);
}
//------------------------------------------------------------------------------
void DecoderUI::ShowTexture(float scale)
{
    if (decoder_ && decoder_->textureID)
    {
        ImTextureID texture = 0;

#if defined(_WIN32)
        DecoderWindows* decoder_win = (DecoderWindows*)decoder_;
        if (decoder_win && decoder_win->srv)
            texture = (ImTextureID)decoder_win->srv;
        else
            texture = (ImTextureID)decoder_win->CreateSRV();
#elif defined(__ANDROID__)
        DecoderAndroid* decoder_android = (DecoderAndroid*)decoder_;
        if (decoder_android && decoder_android->textureID)
            texture = (ImTextureID)(intptr_t)decoder_android->textureID;
#elif defined(__APPLE__)

#endif
        ImGui::Text("Texture ID = 0x%x", texture);
        ImGui::Text("Size = %d x %d", decoder_->texture_width, decoder_->texture_height);

        if (texture)
        {
            ImGui::Image(texture, 
                ImVec2((float)decoder_->texture_width / scale, (float)decoder_->texture_height / scale));
        }
    }
}
//------------------------------------------------------------------------------