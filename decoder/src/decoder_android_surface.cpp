#include "decoder_android_surface.hpp"

#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <media/NdkMediaCodec.h>

#include "logger.hpp"

//------------------------------------------------------------------------------
AndroidSurface::AndroidSurface(Logger* logger, android_app* g_app)
{
    logger_ = logger;
    app_ = g_app;
    app_->activity->vm->AttachCurrentThread(&env_, nullptr);
}
//------------------------------------------------------------------------------
AndroidSurface::~AndroidSurface()
{
    Destroy();
    if (env_ != nullptr)
    {
        app_->activity->vm->DetachCurrentThread();
        env_ = nullptr;
    }
}
//------------------------------------------------------------------------------
ANativeWindow* AndroidSurface::CreateFromTexture(int textureID)
{
    if (!CreateSurfaceTexture(textureID))
    {
        logger_->Log("Failed to create SurfaceTexture");
        return nullptr;
    }

    if (!CreateSurface())
    {
        logger_->Log("Failed to create Surface");
        return nullptr;
    }

    if (!CreateNativeWindow())
    {
        logger_->Log("Failed to create ANativeWindow from Surface");
        return nullptr;
    }

    return nativeWindow_;
}
//------------------------------------------------------------------------------
void AndroidSurface::Destroy()
{
    if (nativeWindow_ != nullptr)
    {
        ANativeWindow_release(nativeWindow_);
        nativeWindow_ = nullptr;
    }
    if (surface_ != nullptr)
    {
        env_->DeleteGlobalRef(surface_);
        surface_ = nullptr;
    }
    if (surfaceTexture_ != nullptr)
    {
        env_->DeleteGlobalRef(surfaceTexture_);
        surfaceTexture_ = nullptr;
    }
}
//------------------------------------------------------------------------------
bool AndroidSurface::CreateSurfaceTexture(int textureID)
{
    jclass surfaceTextureClass = env_->FindClass("android/graphics/SurfaceTexture");
    if (surfaceTextureClass == nullptr)
    {
        logger_->Log("Failed to find SurfaceTexture class");
        return false;
    }

    jmethodID constructor = env_->GetMethodID(surfaceTextureClass, "<init>", "(I)V");
    if (constructor == nullptr)
    {
        logger_->Log("Failed to find SurfaceTexture constructor");
        return false;
    }

    surfaceTexture_ = env_->NewObject(surfaceTextureClass, constructor, textureID);
    if (surfaceTexture_ == nullptr)
    {
        logger_->Log("Failed to create SurfaceTexture object");
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
bool AndroidSurface::CreateSurface()
{
    jclass surfaceClass = env_->FindClass("android/view/Surface");
    if (surfaceClass == nullptr)
    {
        logger_->Log("Failed to find Surface class");
        return false;
    }

    jmethodID constructor = env_->GetMethodID(surfaceClass, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
    if (constructor == nullptr)
    {
        logger_->Log("Failed to find Surface constructor");
        return false;
    }

    surface_ = env_->NewObject(surfaceClass, constructor, surfaceTexture_);
    if (surface_ == nullptr)
    {
        logger_->Log("Failed to create Surface object");
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
bool AndroidSurface::CreateNativeWindow()
{
    if (surface_ == nullptr)
    {
        logger_->Log("Surface is not created");
        return false;
    }

    nativeWindow_ = ANativeWindow_fromSurface(env_, surface_);
    if (nativeWindow_ == nullptr)
    {
        logger_->Log("Failed to create ANativeWindow from Surface");
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
void AndroidSurface::Update()
{
    jclass surfaceTextureClass = env_->GetObjectClass(surfaceTexture_);
    jmethodID updateTexImage = env_->GetMethodID(surfaceTextureClass, "updateTexImage", "()V");
    env_->CallVoidMethod(surfaceTexture_, updateTexImage);
}