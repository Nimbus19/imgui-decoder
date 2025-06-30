#if defined(_WIN32)
#   define NOMINMAX
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#include "decoder.hpp"
#include "decoder_windows.hpp"
#include "decoder_export.h"

//------------------------------------------------------------------------------
Decoder* WINAPI Create()
{
    return new Decoder(nullptr);
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
#pragma comment(linker, "/export:Create=_Create@0")
#pragma comment(linker, "/export:Decode=_Decode@4")
#pragma comment(linker, "/export:Render=_Render@4")
#pragma comment(linker, "/export:Destroy=_Destroy@4")
#endif
//------------------------------------------------------------------------------
