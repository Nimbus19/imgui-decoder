#pragma once

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <wmsdkidl.h>
#include <unordered_map>

struct GuidNameMap 
{
    const GUID* guid;
    const char* name;
};

static const GuidNameMap g_guidNameMap[] = 
{
    { &MF_MT_FRAME_SIZE, "MF_MT_FRAME_SIZE" },
    { &MF_MT_AVG_BITRATE, "MF_MT_AVG_BITRATE" },
    { &MF_MT_MPEG4_SAMPLE_DESCRIPTION, "MF_MT_MPEG4_SAMPLE_DESCRIPTION" },
    { &MF_MT_MPEG_SEQUENCE_HEADER, "MF_MT_MPEG_SEQUENCE_HEADER" },
    { &MF_MT_MAJOR_TYPE, "MF_MT_MAJOR_TYPE" },
    { &MF_MT_AM_FORMAT_TYPE, "MF_MT_AM_FORMAT_TYPE" },
    { &MF_PROGRESSIVE_CODING_CONTENT, "MF_PROGRESSIVE_CODING_CONTENT" },
    { &MF_MT_VIDEO_LEVEL, "MF_MT_VIDEO_LEVEL" },
    { &MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY, "MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY" },
    { &MF_NALU_LENGTH_SET, "MF_NALU_LENGTH_SET" },
    { &MF_MT_VIDEO_PROFILE, "MF_MT_VIDEO_PROFILE" },
    { &MF_MT_VIDEO_NOMINAL_RANGE, "MF_MT_VIDEO_NOMINAL_RANGE" },
    { &MF_MT_VIDEO_ROTATION, "MF_MT_VIDEO_ROTATION" },
    { &MF_MT_FRAME_RATE, "MF_MT_FRAME_RATE" },
    { &MF_MT_PIXEL_ASPECT_RATIO, "MF_MT_PIXEL_ASPECT_RATIO" },
    { &MF_MT_SAMPLE_SIZE, "MF_MT_SAMPLE_SIZE" },
    { &MF_MT_INTERLACE_MODE, "MF_MT_INTERLACE_MODE" },
    { &MF_MT_SUBTYPE, "MF_MT_SUBTYPE" },
    
    { &MFMediaType_Audio, "MFMediaType_Audio" },    
    { &MFMediaType_Video, "MFMediaType_Video" },

    { &MFVideoFormat_H264, "MFVideoFormat_H264" },
    { &MFVideoFormat_H265, "MFVideoFormat_H265" },
    { &MFVideoFormat_HEVC, "MFVideoFormat_HEVC" },
    { &MFVideoFormat_VP90, "MFVideoFormat_VP90" },
    { &MFVideoFormat_AV1, "MFVideoFormat_AV1" },

    { &WMFORMAT_MPEG2Video, "WMFORMAT_MPEG2Video" },

    { &MFVideoFormat_ARGB32, "MFVideoFormat_ARGB32" },
    { &MFVideoFormat_RGB32, "MFVideoFormat_RGB32" },
    { &MFVideoFormat_RGB24, "MFVideoFormat_RGB24" },
    { &MFVideoFormat_RGB555, "MFVideoFormat_RGB555" },
    { &MFVideoFormat_RGB565, "MFVideoFormat_RGB565" },
    { &MFVideoFormat_NV12, "MFVideoFormat_NV12" },
    { &MFVideoFormat_YUY2, "MFVideoFormat_YUY2" },
    { &MFVideoFormat_IYUV, "MFVideoFormat_IYUV" },
    { &MFVideoFormat_YV12, "MFVideoFormat_YV12" },
    
    // ...
};

// Media Foundation Error Code
static const std::unordered_map<HRESULT, const char*> g_mfErrorMap =
{
    { MF_E_TRANSFORM_TYPE_NOT_SET, "Output type not set for this stream." },
    { MF_E_TRANSFORM_STREAM_CHANGE, "Stream change: output type or format changed." },
    { MF_E_TRANSFORM_NEED_MORE_INPUT, "Transform needs more input before producing output." },
};
