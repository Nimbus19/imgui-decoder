#pragma once

typedef enum OMX_COLOR_FORMATTYPE {
    OMX_COLOR_FormatUnused,
    OMX_COLOR_FormatMonochrome,
    OMX_COLOR_Format8bitRGB332,
    OMX_COLOR_Format12bitRGB444,
    OMX_COLOR_Format16bitARGB4444,
    OMX_COLOR_Format16bitARGB1555,
    OMX_COLOR_Format16bitRGB565,
    OMX_COLOR_Format16bitBGR565,
    OMX_COLOR_Format18bitRGB666,
    OMX_COLOR_Format18bitARGB1665,
    OMX_COLOR_Format19bitARGB1666,
    OMX_COLOR_Format24bitRGB888,
    OMX_COLOR_Format24bitBGR888,
    OMX_COLOR_Format24bitARGB1887,
    OMX_COLOR_Format25bitARGB1888,
    OMX_COLOR_Format32bitBGRA8888,
    OMX_COLOR_Format32bitARGB8888,
    OMX_COLOR_FormatYUV411Planar,
    OMX_COLOR_FormatYUV411PackedPlanar,
    OMX_COLOR_FormatYUV420Planar,           // 0x13 (I420, YV12)
    OMX_COLOR_FormatYUV420PackedPlanar,
    OMX_COLOR_FormatYUV420SemiPlanar,       // 0x15 (NV12, NV21)
    OMX_COLOR_FormatYUV422Planar,
    OMX_COLOR_FormatYUV422PackedPlanar,
    OMX_COLOR_FormatYUV422SemiPlanar,
    OMX_COLOR_FormatYCbYCr,                 // 0x19 (YUY2, UYVY)
    OMX_COLOR_FormatYCrYCb,                 // 0x1A (YVYU, VYUY)
    OMX_COLOR_FormatCbYCrY,                 // 0x1B (YV12, IYUV)
    OMX_COLOR_FormatCrYCbY,                 // 0x1C (YVU9, YV24)
    OMX_COLOR_FormatYUV444Interleaved,
    OMX_COLOR_FormatRawBayer8bit,
    OMX_COLOR_FormatRawBayer10bit,
    OMX_COLOR_FormatRawBayer8bitcompressed,
    OMX_COLOR_FormatL2,
    OMX_COLOR_FormatL4,
    OMX_COLOR_FormatL8,
    OMX_COLOR_FormatL16,
    OMX_COLOR_FormatL24,
    OMX_COLOR_FormatL32,
    OMX_COLOR_FormatYUV420PackedSemiPlanar,
    OMX_COLOR_FormatYUV422PackedSemiPlanar,
    OMX_COLOR_Format18BitBGR666,
    OMX_COLOR_Format24BitARGB6666,
    OMX_COLOR_Format24BitABGR6666,
    OMX_COLOR_FormatKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
    OMX_COLOR_FormatVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    /**<Reserved android opaque colorformat. Tells the encoder that
     * the actual colorformat will be  relayed by the
     * Gralloc Buffers.
     * FIXME: In the process of reserving some enum values for
     * Android-specific OMX IL colorformats. Change this enum to
     * an acceptable range once that is done.
     * */
    OMX_COLOR_FormatAndroidOpaque = 0x7F000789,
    OMX_COLOR_Format32BitRGBA8888 = 0x7F00A000,
    /** Flexible 8-bit YUV format.  Codec should report this format
     *  as being supported if it supports any YUV420 packed planar
     *  or semiplanar formats.  When port is set to use this format,
     *  codec can substitute any YUV420 packed planar or semiplanar
     *  format for it. */
    OMX_COLOR_FormatYUV420Flexible = 0x7F420888,
    // 10-bit or 12-bit YUV format, LSB-justified (0's on higher bits)
    OMX_COLOR_FormatYUV420Planar16 = 0x7F42016B,
    // Packed 10-bit YUV444 representation that includes 2 bits of alpha. Each pixel is
    // 32-bit. Bits 0-9 contain the U sample, bits 10-19 contain the Y sample,
    // bits 20-29 contain the V sample, and bits 30-31 contain the alpha value.
    OMX_COLOR_FormatYUV444Y410 = 0x7F444AAA,
    OMX_TI_COLOR_FormatYUV420PackedSemiPlanar = 0x7F000100,
    OMX_QCOM_COLOR_FormatYVU420SemiPlanar = 0x7FA30C00,
    OMX_QCOM_COLOR_FormatYUV420PackedSemiPlanar64x32Tile2m8ka = 0x7FA30C03,
    OMX_SEC_COLOR_FormatNV12Tiled = 0x7FC00002,
    OMX_QCOM_COLOR_FormatYUV420PackedSemiPlanar32m = 0x7FA30C04,
    OMX_COLOR_FormatMax = 0x7FFFFFFF
} OMX_COLOR_FORMATTYPE;
