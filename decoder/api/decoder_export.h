#pragma once

#ifdef _WIN32
#  define LIB_EXPORT __declspec(dllexport)
#  define WINAPI __stdcall
#else
#  define LIB_EXPORT __attribute__((visibility("default")))
#  define WINAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

LIB_EXPORT class Decoder* WINAPI CreateWIN(struct ID3D11Device* d3d_device, struct ID3D11DeviceContext* d3d_context);
LIB_EXPORT bool WINAPI Decode(class Decoder*);
LIB_EXPORT bool WINAPI Render(class Decoder*);
LIB_EXPORT void WINAPI Destroy(class Decoder*);

#ifdef __cplusplus
}
#endif
