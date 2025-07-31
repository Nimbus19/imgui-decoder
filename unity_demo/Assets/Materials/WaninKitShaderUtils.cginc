// ------------------------------------------------------------
// WaninKit Utility Functions
// Author: Wythe
// ------------------------------------------------------------

#ifndef WANINKIT_UTILS_INCLUDED
#define WANINKIT_UTILS_INCLUDED

// YUV 到 RGB 轉換
float3 YUVToRGB(float3 yuvColor)
{
    float3 yuv;
    yuv.r = yuvColor.r;
    yuv.g = yuvColor.g - 0.5;
    yuv.b = yuvColor.b - 0.5;
    
    float3 rgb;
    rgb.r = yuv.r +                     yuv.b *  1.581000;
    rgb.g = yuv.r + yuv.g * -0.188062 + yuv.b * -0.469967;
    rgb.b = yuv.r + yuv.g *  1.862906;
    
    return rgb;
}

#endif // WANINKIT_UTILS_INCLUDED
