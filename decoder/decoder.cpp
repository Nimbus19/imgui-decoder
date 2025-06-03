#include "decoder.hpp"
#include "imgui.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(__ANDROID__)
#   include <android/log.h>
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
void Decoder::DrawUI()
{
    ImGui::SetNextWindowSize(ImVec2(1280, 640), ImGuiCond_Once);
    ImGui::Begin("Decoder");
    ImGui::Text("Decode video");
    if (ImGui::Button("ReadFile"))
    {
        ReadFile();
    }
    if (ImGui::Button("CreateTexture"))
    {

        uint8_t* data = (uint8_t*)malloc(width * height * 4);
        for (int i = 0; i < width * height; ++i)
        {
            data[i * 4 + 0] = 255; // R
            data[i * 4 + 1] = 255;   // G
            data[i * 4 + 2] = 0;   // B
            data[i * 4 + 3] = 255; // A
        }
        textureID = CreateTexture(width, height, data);
        free(data);
    }
    if (textureID != 0)
    {
        ImGui::Text("OpenGL Texture:");
        ImGui::Text("pointer = %x", textureID);
        ImGui::Text("size = %d x %d", width, height);
        ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(width, height));
    }
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