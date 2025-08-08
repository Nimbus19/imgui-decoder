#pragma once

#include <jni.h>

class AndroidSurface
{
public:
    explicit AndroidSurface(class Logger* logger, struct android_app* g_app);
    virtual ~AndroidSurface();

    struct ANativeWindow* CreateFromTexture(int textureID);
    void Destroy();
    void Update();

private:
    struct android_app* app_ = nullptr;
    class Logger* logger_ = nullptr;
    JNIEnv* env_ = nullptr;
    jobject surfaceTexture_ = nullptr;
    jobject surface_ = nullptr;
    struct ANativeWindow* nativeWindow_ = nullptr;

    bool CreateSurfaceTexture();
    bool CreateSurfaceTexture(int textureID);
    bool CreateSurface();
    bool CreateNativeWindow();
};