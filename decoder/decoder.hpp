#pragma once

class Decoder
{
public:
    Decoder(struct android_app* app);
    virtual ~Decoder();

    void DrawUI();
    bool ReadFile();

private:
    struct android_app* m_App;

    static void log(const char* fmt, ...);
};