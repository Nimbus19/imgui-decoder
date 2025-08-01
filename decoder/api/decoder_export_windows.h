#pragma once

#include <cstdint>
#include "decoder_export.h"

#ifdef __cplusplus
extern "C" {
#endif

LIB_EXPORT class Decoder* WINAPI CreateWIN(struct ID3D11Device* d3d_device, struct ID3D11DeviceContext* d3d_context);
LIB_EXPORT bool WINAPI DecodeWIN(class Decoder*);
LIB_EXPORT bool WINAPI RenderWIN(class Decoder*);
LIB_EXPORT void WINAPI DestroyWIN(class Decoder*);
LIB_EXPORT intptr_t WINAPI GetTextureWIN(class Decoder*);

LIB_EXPORT struct ID3D11Device* WINAPI GetD3D11Device();
LIB_EXPORT struct ID3D11DeviceContext* WINAPI GetD3D11Context();

#ifdef __cplusplus
}
#endif
