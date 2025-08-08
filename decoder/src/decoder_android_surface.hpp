#pragma once

#ifdef __ANDROID__
#include <jni.h>
#else
struct _JNIEnv; typedef _JNIEnv JNIEnv; struct _JavaVM; typedef _JavaVM JavaVM; // fallback for non-Android IntelliSense
#endif

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
    JavaVM* vm_ = nullptr; // store JavaVM, fetch JNIEnv per-thread
    jobject surfaceTexture_ = nullptr;
    jobject surface_ = nullptr;
    struct ANativeWindow* nativeWindow_ = nullptr;

    JNIEnv* GetEnv();
    bool CreateSurfaceTexture(int textureID);
    bool CreateSurface();
    bool CreateNativeWindow();
};