#if defined(_WIN32)
#   define NOMINMAX
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#include "decoder.hpp"
#include "decoder_windows.hpp"
#include "decoder_export.h"

//------------------------------------------------------------------------------
Decoder* WINAPI CreateWIN(struct ID3D11Device* d3d_device, struct ID3D11DeviceContext* d3d_context)
{
    auto decoder = new DecoderWindows(nullptr, d3d_device, d3d_context, nullptr);
    return decoder;
}
//------------------------------------------------------------------------------
bool WINAPI Decode(Decoder* decoder)
{
    return false;
}
//------------------------------------------------------------------------------
bool WINAPI Render(Decoder* decoder)
{
    return false;
}
//------------------------------------------------------------------------------
void WINAPI Destroy(Decoder* decoder)
{
    if (decoder)
    {
        delete decoder;
        decoder = nullptr;
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Link symbol
//------------------------------------------------------------------------------
#if defined(_M_IX86)
#pragma comment(linker, "/export:CreateWIN=_CreateWIN@8")
#pragma comment(linker, "/export:Decode=_Decode@4")
#pragma comment(linker, "/export:Render=_Render@4")
#pragma comment(linker, "/export:Destroy=_Destroy@4")
#endif
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Unity Native Plugins
//------------------------------------------------------------------------------
#include <d3d11.h>
#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"
#include "IUnityGraphicsD3D12.h"

static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphicsD3D11* s_D3D11 = nullptr;
static IUnityGraphicsD3D12* s_D3D12 = nullptr;

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    s_UnityInterfaces = unityInterfaces;
    s_D3D11 = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
    s_D3D12 = s_UnityInterfaces->Get<IUnityGraphicsD3D12>();
}

extern "C" ID3D11Device* GetD3D11Device()
{
    if (s_D3D11)
        return s_D3D11->GetDevice();
    return nullptr;
}

extern "C" ID3D11DeviceContext* GetD3D11Context()
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