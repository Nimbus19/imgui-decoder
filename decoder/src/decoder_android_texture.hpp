#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class AndroidTexture
{
public:
    explicit AndroidTexture(class Logger* logger, struct android_app* g_app);
    virtual ~AndroidTexture();

    bool CreateGLTexture(int texW, int texH);
    void DestroyGLTexture();
    bool UpdateGLTexture(const void* data);

    bool CreateOESTexture();
    void DestroyOESTexture();

    // 將當前 OES 紋理內容渲染到 glTex（2D 紋理），以便 ImGui 顯示。
    bool RenderOESToGL();

    GLuint glTex = 0;
    GLuint oesTex = 0;
    int width = 0;
    int height = 0;

private:
    // 初始化/銷毀渲染管線（FBO + Program），內部自動延遲初始化
    bool EnsureOesToGlPipeline();
    void DestroyOesToGlPipeline();

    // GL 資源
    GLuint fbo_ = 0;
    GLuint program_ = 0;
    GLint attr_pos_ = -1;
    GLint attr_uv_ = -1;
    GLint uni_sampler_ = -1;

    struct android_app* app_ = nullptr;
    class Logger* logger_ = nullptr;
};