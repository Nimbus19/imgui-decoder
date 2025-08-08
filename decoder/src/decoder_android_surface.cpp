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
    vm_ = app_->activity->vm; // store JavaVM
}
//------------------------------------------------------------------------------
AndroidSurface::~AndroidSurface()
{
    Destroy();
    // Do not force Detach here; long-lived native threads can stay attached.
}
//------------------------------------------------------------------------------
JNIEnv* AndroidSurface::GetEnv()
{
    if (!vm_) return nullptr;
    JNIEnv* env = nullptr;
    jint res = vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (res == JNI_EDETACHED)
    {
        if (vm_->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            logger_->Log("AttachCurrentThread failed\n");
            return nullptr;
        }
    }
    else if (res != JNI_OK)
    {
        logger_->Log("GetEnv failed\n");
        return nullptr;
    }
    return env;
}
//------------------------------------------------------------------------------
ANativeWindow* AndroidSurface::CreateFromTexture(int textureID)
{
    if (!CreateSurfaceTexture(textureID))
    {
        logger_->Log("Failed to create SurfaceTexture\n");
        return nullptr;
    }

    if (!CreateSurface())
    {
        logger_->Log("Failed to create Surface\n");
        return nullptr;
    }

    if (!CreateNativeWindow())
    {
        logger_->Log("Failed to create ANativeWindow from Surface\n");
        return nullptr;
    }

    return nativeWindow_;
}
//------------------------------------------------------------------------------
void AndroidSurface::Destroy()
{
    JNIEnv* env = GetEnv();
    if (nativeWindow_ != nullptr)
    {
        ANativeWindow_release(nativeWindow_);
        nativeWindow_ = nullptr;
    }
    if (env && surface_ != nullptr)
    {
        env->DeleteGlobalRef(surface_);
        surface_ = nullptr;
    }
    if (env && surfaceTexture_ != nullptr)
    {
        jclass surfaceTextureClass = env->GetObjectClass(surfaceTexture_);
        jmethodID releaseMethod = env->GetMethodID(surfaceTextureClass, "release", "()V");
        if (releaseMethod)
            env->CallVoidMethod(surfaceTexture_, releaseMethod);
        if (env->ExceptionCheck())
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteGlobalRef(surfaceTexture_);
        surfaceTexture_ = nullptr;
    }
}
//------------------------------------------------------------------------------
bool AndroidSurface::CreateSurfaceTexture(int textureID)
{
    JNIEnv* env = GetEnv();
    if (!env) return false;

    jclass surfaceTextureClass = env->FindClass("android/graphics/SurfaceTexture");
    if (surfaceTextureClass == nullptr)
    {
        logger_->Log("Failed to find SurfaceTexture class\n");
        return false;
    }

    jmethodID constructor = env->GetMethodID(surfaceTextureClass, "<init>", "(I)V");
    if (constructor == nullptr)
    {
        logger_->Log("Failed to find SurfaceTexture constructor\n");
        return false;
    }

    jobject localObj = env->NewObject(surfaceTextureClass, constructor, textureID);
    if (localObj == nullptr)
    {
        logger_->Log("Failed to create SurfaceTexture object\n");
        return false;
    }
    surfaceTexture_ = env->NewGlobalRef(localObj);

    return true;
}
//------------------------------------------------------------------------------
bool AndroidSurface::CreateSurface()
{
    JNIEnv* env = GetEnv();
    if (!env) return false;

    jclass surfaceClass = env->FindClass("android/view/Surface");
    if (surfaceClass == nullptr)
    {
        logger_->Log("Failed to find Surface class\n");
        return false;
    }

    jmethodID constructor = env->GetMethodID(surfaceClass, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
    if (constructor == nullptr)
    {
        logger_->Log("Failed to find Surface constructor\n");
        return false;
    }

    jobject localObj = env->NewObject(surfaceClass, constructor, surfaceTexture_);
    if (localObj == nullptr)
    {
        logger_->Log("Failed to create Surface object\n");
        return false;
    }
    surface_ = env->NewGlobalRef(localObj);

    return true;
}
//------------------------------------------------------------------------------
bool AndroidSurface::CreateNativeWindow()
{
    JNIEnv* env = GetEnv();
    if (!env) return false;
    nativeWindow_ = ANativeWindow_fromSurface(env, surface_);
    if (nativeWindow_ == nullptr)
    {
        logger_->Log("Failed to create ANativeWindow from Surface\n");
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
void AndroidSurface::Update()
{
    // Must be called on GL thread that owns the external OES texture's EGLContext
    JNIEnv* env = GetEnv();
    if (!env || !surfaceTexture_) return;
    jclass surfaceTextureClass = env->GetObjectClass(surfaceTexture_);
    jmethodID updateTexImage = env->GetMethodID(surfaceTextureClass, "updateTexImage", "()V");
    if (!updateTexImage) return;
    env->CallVoidMethod(surfaceTexture_, updateTexImage);
    if (env->ExceptionCheck())
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}