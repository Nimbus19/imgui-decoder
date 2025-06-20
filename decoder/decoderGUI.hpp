#pragma once

#include <cstdint>

class DecoderGUI
{
public:
    const char kVersion[8] = "v 1.1";

    explicit DecoderGUI(struct android_app* app);
    virtual ~DecoderGUI();

    void DrawUI();
    void DrawReadUI();
    void DrawDecodeUI();
    void DrawRenderUI();

protected:
    static void Log(const char* fmt, ...);

    struct android_app* app_ = nullptr;
    class Decoder* decoder_ = nullptr;

    int curr_page_ = 0; // 0: Read, 1: Decode, 2: Render

    // Read
    char output_path_[256] = "/sdcard/Download/apple_bipbop/bipbop.mp4";
    char file_buffer_[1024] = "FileBuffer";
};