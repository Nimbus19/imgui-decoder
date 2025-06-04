#include "decoder.hpp"
#include "imgui.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(__ANDROID__)
#   include <android/log.h>
#   include <media/NdkMediaCodec.h>
#   include <media/NdkMediaExtractor.h>
#endif

//------------------------------------------------------------------------------
Decoder::Decoder(android_app* app)
{
    m_App = app;
}
//------------------------------------------------------------------------------
Decoder::~Decoder()
{
    DestroyTexture(textureID);
}
//------------------------------------------------------------------------------
//void Decoder::DrawUI()
//{
//    ImGui::SetNextWindowSize(ImVec2(1280, 640), ImGuiCond_Once);
//    ImGui::Begin("Decoder");
//    ImGui::Text("Decode video");
//    if (ImGui::Button("ReadFile"))
//    {
//        ReadFile();
//    }
//    if (ImGui::Button("CreateTexture"))
//    {
//
//        uint8_t* data = (uint8_t*)malloc(width * height * 4);
//        for (int i = 0; i < width * height; ++i)
//        {
//            data[i * 4 + 0] = 255; // R
//            data[i * 4 + 1] = 255;   // G
//            data[i * 4 + 2] = 0;   // B
//            data[i * 4 + 3] = 255; // A
//        }
//        textureID = CreateTexture(width, height, data);
//        free(data);
//    }
//    if (textureID != 0)
//    {
//        ImGui::Text("OpenGL Texture:");
//        ImGui::Text("pointer = %x", textureID);
//        ImGui::Text("size = %d x %d", width, height);
//        ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(width, height));
//    }
//    ImGui::End();
//}
//------------------------------------------------------------------------------
void Decoder::DrawUI()
{
    // 設定視窗大小
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    // 左側功能選單
    ImGui::BeginChild("Sidebar", ImVec2(220, 0), true, ImGuiWindowFlags_NoScrollbar);
    {

        if (ImGui::Button("Read", ImVec2(-1, 40))) {/*...*/}
        ImGui::Spacing();
        if (ImGui::Button("Decode", ImVec2(-1, 40))) {/*...*/}
        ImGui::Spacing();
        if (ImGui::Button("Render", ImVec2(-1, 40))) {/*...*/}

        // 版本顯示
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
        ImGui::Text("v 1.1");
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 右側主作業區
    ImGui::BeginChild("Panel", ImVec2(0, 0), true);
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
        ImGui::InputText("OutPath", outputPath, IM_ARRAYSIZE(outputPath));
        // 檔名前綴
        ImGui::InputText("Prefix", prefix, IM_ARRAYSIZE(prefix));

        // 導出格式
        ImGui::Text("Format:");
        ImGui::RadioButton("Mp4", &exportFormat, 0); ImGui::SameLine();
        ImGui::RadioButton("M4e", &exportFormat, 1); ImGui::SameLine();
        ImGui::RadioButton("Mp4 & M4e", &exportFormat, 2);
        ImGui::Spacing();

        // 全部重設按鈕（紅色）
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f,0.2f,0.2f,1.0f));
        if (ImGui::Button("Reset", ImVec2(120, 40))) {/*...*/}
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // 開始按鈕（綠色）
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.62f,0.88f,0.72f,1.0f));
        if (ImGui::Button("Start", ImVec2(120, 40))) {/*...*/}
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::End();
}
//------------------------------------------------------------------------------
bool Decoder::ReadFile()
{
    const char* filePath = "/sdcard/Download/apple_bipbop/bipbop.mp4";
    FILE* file = fopen(filePath, "r");
    if (file == nullptr)
    {
        Log("Failed to open file");
        return false;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file))
    {
        Log("%s", buffer);
    }

    fclose(file);
    return true;
}
//------------------------------------------------------------------------------
GLuint Decoder::CreateTexture(int width, int height, const void* data)
{
    // create texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // send data to GPU memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    return textureID;
}
//------------------------------------------------------------------------------
void Decoder::DestroyTexture(GLuint textureID)
{
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}
//------------------------------------------------------------------------------
void Decoder::Log(const char *fmt, ...)
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
//------------------------------------------------------------------------------
bool Decoder::GetMediaFormat(AMediaFormat*& outMediaFormat, const char* url, const char* mine)
{
//    FILE *fp;
//    fp = fopen(url, "r");
//    if (fp == nullptr)
//        return false;
//
//    int fd = fileno(fp);
//    fseek(fp, 0, SEEK_END);
//    off64_t size = ftell(fp);
//
//    bool result = false;
//    switch (0) case 0: default:
//    {
//        AMediaExtractor *mediaExtractor = AMediaExtractor_new();
//        media_status_t hr = AMediaExtractor_setDataSourceFd(mediaExtractor, fd, 0, size);
//        size_t trackCount = AMediaExtractor_getTrackCount(mediaExtractor);
//
//        if (hr != AMEDIA_OK or trackCount == 0)
//        {
//            result = false;
//            break;
//        }
//
//        for (int i = 0; i < trackCount; i++)
//        {
//            AMediaFormat* trackFormat = AMediaExtractor_getTrackFormat(mediaExtractor, i);
//            const char* trackMine = "";
//            AMediaFormat_getString(trackFormat, "mime", &trackMine);
//
//            if (strncmp(mine, trackMine, strlen(mine)) == 0)
//            {
//                AMediaFormat_delete(outMediaFormat);
//                outMediaFormat = trackFormat;
//                result = true;
//                break;
//            }
//
//            AMediaFormat_delete(trackFormat);
//        }
//    }
//
//    fclose(fp);
//    return result;
    return false;
}