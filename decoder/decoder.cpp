#include "decoder.hpp"
#include "imgui.h"
#include <stdio.h>
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

}
//------------------------------------------------------------------------------
void Decoder::DrawUI()
{
    ImGui::Begin("Decoder");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Decode video");
    if (ImGui::Button("Decode"))
    {
        ReadFile();
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
        log("Failed to open file");
        return false;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file))
    {
        log("%s", buffer);
    }

    fclose(file);
    return true;
}
//------------------------------------------------------------------------------
void Decoder::log(const char *fmt, ...)
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