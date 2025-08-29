#include "decoder_android_texture.hpp"

#include "logger.hpp"

#define TEXTURE_FORMAT GL_LUMINANCE

// 靜態工具：著色器編譯/鏈接
//------------------------------------------------------------------------------
static GLuint CompileShader(GLenum type, const char* src, Logger* logger)
{
    GLuint sh = glCreateShader(type);
    if (!sh) return 0;
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        GLsizei len = 0;
        glGetShaderInfoLog(sh, sizeof(log), &len, log);
        if (logger) logger->Log("Shader compile failed: %s\n", log);
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}
//------------------------------------------------------------------------------
static GLuint LinkProgram(GLuint vs, GLuint fs, Logger* logger)
{
    GLuint prog = glCreateProgram();
    if (!prog) return 0;
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        GLsizei len = 0;
        glGetProgramInfoLog(prog, sizeof(log), &len, log);
        if (logger) logger->Log("Program link failed: %s\n", log);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}
//------------------------------------------------------------------------------
AndroidTexture::AndroidTexture(Logger* logger, android_app* g_app)
{
    logger_ = logger;
    app_ = g_app;
}
//------------------------------------------------------------------------------
AndroidTexture::~AndroidTexture()
{
    DestroyGLTexture();
    DestroyOESTexture();
    DestroyOesToGlPipeline();
}
//------------------------------------------------------------------------------
bool AndroidTexture::CreateGLTexture(int texW, int texH)
{
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 TEXTURE_FORMAT, texW, texH, 0,
                 TEXTURE_FORMAT, GL_UNSIGNED_BYTE, nullptr);

    width = texW;
    height = texH;

    return true;
}
//------------------------------------------------------------------------------
void AndroidTexture::DestroyGLTexture()
{
    if (glTex != 0)
    {
        glDeleteTextures(1, &glTex);
        glTex = 0;
    }
}
//------------------------------------------------------------------------------
bool AndroidTexture::UpdateGLTexture(const void* data)
{
    if (glTex == 0)
    {
        logger_->Log("Texture not created yet\n");
        return false;
    }
    if (data == nullptr)
    {
        logger_->Log("Data pointer is null\n");
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, glTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    width, height,
                    TEXTURE_FORMAT, GL_UNSIGNED_BYTE,
                    data); // Update texture with new data
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    return true;
}
//------------------------------------------------------------------------------
bool AndroidTexture::CreateOESTexture()
{
    glGenTextures(1, &oesTex);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, oesTex);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}
//------------------------------------------------------------------------------
void AndroidTexture::DestroyOESTexture()
{
    if (oesTex != 0)
    {
        glDeleteTextures(1, &oesTex);
        oesTex = 0;
    }
}
//------------------------------------------------------------------------------
bool AndroidTexture::EnsureOesToGlPipeline()
{
    if (program_ != 0 && fbo_ != 0)
        return true;

    // 構建 shader program
    static const char* vsSrc = R"(
        attribute vec2 aPos;
        attribute vec2 aUV;
        varying vec2 vUV;
        void main(){
            vUV = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";
    static const char* fsSrc = R"(
        #extension GL_OES_EGL_image_external : require
        precision mediump float;
        uniform samplerExternalOES uTex;
        varying vec2 vUV;
        void main(){
            gl_FragColor = texture2D(uTex, vUV);
        }
    )";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSrc, logger_);
    if (!vs) return false;
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc, logger_);
    if (!fs) { glDeleteShader(vs); return false; }

    program_ = LinkProgram(vs, fs, logger_);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!program_) return false;

    attr_pos_ = glGetAttribLocation(program_, "aPos");
    attr_uv_ = glGetAttribLocation(program_, "aUV");
    uni_sampler_ = glGetUniformLocation(program_, "uTex");

    if (attr_pos_ < 0 || attr_uv_ < 0 || uni_sampler_ < 0)
    {
        if (logger_) logger_->Log("Failed to get attrib/uniform locations. pos=%d uv=%d sampler=%d\n", attr_pos_, attr_uv_, uni_sampler_);
        glDeleteProgram(program_); program_ = 0;
        return false;
    }

    glGenFramebuffers(1, &fbo_);
    if (!fbo_)
    {
        if (logger_) logger_->Log("Failed to create FBO\n");
        glDeleteProgram(program_); program_ = 0;
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
void AndroidTexture::DestroyOesToGlPipeline()
{
    if (fbo_)
    {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }
    if (program_)
    {
        glDeleteProgram(program_);
        program_ = 0;
    }
    attr_pos_ = -1;
    attr_uv_ = -1;
    uni_sampler_ = -1;
}

//------------------------------------------------------------------------------
bool AndroidTexture::RenderOESToGL()
{
    if (oesTex == 0 || glTex == 0)
    {
        if (logger_) logger_->Log("RenderOESToGL: missing texture (oesTex=%u glTex=%u)\n", (unsigned)oesTex, (unsigned)glTex);
        return false;
    }
    if (!EnsureOesToGlPipeline())
        return false;

    // 保存 GL 狀態
    GLint prevFbo = 0; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    GLint prevProgram = 0; glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    GLint prevViewport[4]; glGetIntegerv(GL_VIEWPORT, prevViewport);

    GLint prevActiveTex = 0; glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);
    glActiveTexture(GL_TEXTURE0);
    GLint prevTex2D0 = 0; glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex2D0);
    GLint prevTexOES0 = 0; glGetIntegerv(GL_TEXTURE_BINDING_EXTERNAL_OES, &prevTexOES0);

    // 綁定 FBO，嘗試直接附著 glTex
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTex, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    bool needCopy = false;
    GLuint tempTex = 0;
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        // glTex 可能是 GL_LUMINANCE，無法直接附著，改用臨時 RGBA 紋理
        needCopy = true;
        glGenTextures(1, &tempTex);
        glBindTexture(GL_TEXTURE_2D, tempTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTex, 0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            if (logger_) logger_->Log("RenderOESToGL: FBO incomplete (status=0x%04x)\n", status);
            // 還原狀態
            glBindTexture(GL_TEXTURE_2D, prevTex2D0);
            if (tempTex) glDeleteTextures(1, &tempTex);
            glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
            glUseProgram(prevProgram);
            glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, prevTexOES0);
            glActiveTexture(prevActiveTex);
            return false;
        }
    }

    // 設置 viewport
    glViewport(0, 0, width, height);

    // program + uniforms
    glUseProgram(program_);
    glUniform1i(uni_sampler_, 0);

    // 綁定 external OES 紋理到單元 0
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, oesTex);

    // 頂點/UV（覆蓋整個目標）
    const GLfloat verts[8] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    }; // TRIANGLE_STRIP
    const GLfloat uvs[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    glEnableVertexAttribArray(attr_pos_);
    glEnableVertexAttribArray(attr_uv_);
    glVertexAttribPointer(attr_pos_, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_uv_, 2, GL_FLOAT, GL_FALSE, 0, uvs);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // 如需拷貝到 glTex（臨時目標 -> glTex）
    if (needCopy)
    {
        // 仍然綁定著 FBO（附著了 tempTex），直接從 FBO 拷貝到 glTex
        glBindTexture(GL_TEXTURE_2D, glTex);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    }

    // 清理臨時
    if (tempTex)
    {
        // 解除附著以避免刪除時仍在使用
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glDeleteTextures(1, &tempTex);
    }

    // 還原狀態
    glDisableVertexAttribArray(attr_pos_);
    glDisableVertexAttribArray(attr_uv_);

    glBindTexture(GL_TEXTURE_2D, prevTex2D0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, prevTexOES0);
    glActiveTexture(prevActiveTex);

    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glUseProgram(prevProgram);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    return true;
}
