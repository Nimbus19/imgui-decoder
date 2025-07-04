#include "logger.hpp"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>

#if defined(_WIN32)
#   include <windows.h>
#elif defined(__ANDROID__)
#   include <android/log.h>
#elif defined(__APPLE__)
#endif

//------------------------------------------------------------------------------
Logger::Logger()
{

}
//------------------------------------------------------------------------------
Logger::~Logger()
{

}
//------------------------------------------------------------------------------
void Logger::Log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(fmt, args);
    va_end(args);
}
//------------------------------------------------------------------------------
void Logger::Log(const char* fmt, va_list args)
{
    ConsoleLog(fmt, args);
    for (size_t i = 0; i < outputs_.size(); ++i)
    {
        char* buffer = outputs_[i];
        size_t buffer_size = output_sizes_[i];
        if (buffer && buffer_size > 0)
        {
            size_t curr_len = strlen(buffer);
            size_t remain = buffer_size > curr_len ? buffer_size - curr_len : 0;
            if (remain > 1)
            {
                vsnprintf(buffer + curr_len, remain - 1, fmt, args);
            }
        }
    }
}
//------------------------------------------------------------------------------
void Logger::AddOutput(char* buffer, size_t buffer_size)
{
    if (buffer && buffer_size > 0)
    {
        outputs_.push_back(buffer);
        output_sizes_.push_back(buffer_size);
    }
}
//------------------------------------------------------------------------------
void Logger::RemoveOutput(char* buffer)
{
    for (size_t i = 0; i < outputs_.size(); ++i)
    {
        if (outputs_[i] == buffer)
        {
            outputs_.erase(outputs_.begin() + i);
            output_sizes_.erase(output_sizes_.begin() + i);
            break;
        }
    }
}
//------------------------------------------------------------------------------
void Logger::ClearOutputs()
{ 
    outputs_.clear(); 
    output_sizes_.clear(); 
}
//------------------------------------------------------------------------------
void Logger::ConsoleLog(const char* fmt, va_list args)
{
#if defined(_WIN32)
    char temp[4096];
    vsnprintf(temp, 4096, fmt, args);
    OutputDebugStringA(temp);
#elif defined(__ANDROID__)
    __android_log_vprint(ANDROID_LOG_INFO, "Decoder", fmt, args);
#elif defined(__APPLE__)

#endif
}
//------------------------------------------------------------------------------