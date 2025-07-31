#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "decoder_export_windows.h"

#include <windows.h>
#include <d3d11.h>
#include <d3d12.h>

#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"
#include "Unity/IUnityGraphicsD3D11.h"
#include "Unity/IUnityGraphicsD3D12.h"

#include "decoder.hpp"
#include "decoder_windows.hpp"

static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphicsD3D11* s_D3D11 = nullptr;
static IUnityGraphicsD3D12* s_D3D12 = nullptr;

//------------------------------------------------------------------------------
Decoder* WINAPI CreateWIN(struct ID3D11Device* d3d_device, struct ID3D11DeviceContext* d3d_context)
{
    DecoderWindows* decoder = new DecoderWindows(nullptr, d3d_device, d3d_context, nullptr);
    decoder->ReadMedia("E:\\Videos\\apple_bipbop\\bipbop.mp4");
    decoder->CreateCodec();
    return decoder;
}
//------------------------------------------------------------------------------
bool WINAPI DecodeWIN(Decoder* decoder)
{
    if (!decoder)
        return false;
    return decoder->DecodeFrame();
}
//------------------------------------------------------------------------------
bool WINAPI RenderWIN(Decoder* decoder)
{
    if (!decoder)
        return false;
    return decoder->RenderFrame();
}
//------------------------------------------------------------------------------
void WINAPI DestroyWIN(Decoder* decoder)
{
    if (decoder)
    {
        delete decoder;
        decoder = nullptr;
    }
}
//------------------------------------------------------------------------------
intptr_t WINAPI GetTextureWIN(Decoder* decoder)
{
    if (decoder)
    {
        return decoder->textureID;
    }
    return 0;
}
//------------------------------------------------------------------------------
ID3D11Device* WINAPI GetD3D11Device()
{
    if (s_D3D11)
        return s_D3D11->GetDevice();
    return nullptr;
}
//------------------------------------------------------------------------------
ID3D11DeviceContext* WINAPI GetD3D11Context()
{
    if (s_D3D11)
    {
        ID3D11Device* device = s_D3D11->GetDevice();
        if (device)
        {
            ID3D11DeviceContext* context = nullptr;
            device->GetImmediateContext(&context);
            return context;
        }
    }
    return nullptr;
}
//------------------------------------------------------------------------------
// Link symbol
//------------------------------------------------------------------------------
#if defined(_M_IX86)
#   pragma comment(linker, "/export:CreateWIN=_CreateWIN@8")
#   pragma comment(linker, "/export:DecodeWIN=_DecodeWIN@4")
#   pragma comment(linker, "/export:RenderWIN=_RenderWIN@4")
#   pragma comment(linker, "/export:DestroyWIN=_DestroyWIN@4")
#   pragma comment(linker, "/export:GetTextureWIN=_GetTextureWIN@4")
#   pragma comment(linker, "/export:GetD3D11Device=_GetD3D11Device@0")
#   pragma comment(linker, "/export:GetD3D11Context=_GetD3D11Context@0")
#endif
//------------------------------------------------------------------------------
// Unity Native Plugins
//------------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    s_UnityInterfaces = unityInterfaces;
    s_D3D11 = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
    s_D3D12 = s_UnityInterfaces->Get<IUnityGraphicsD3D12>();
}
//------------------------------------------------------------------------------