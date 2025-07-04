#include "decoder.hpp"

#include <cstdarg>

#include "logger.hpp"

//------------------------------------------------------------------------------
Decoder::Decoder(Logger* logger)
{
#ifdef _DEBUG
    if (logger == nullptr)
    {
        logger_ = new Logger();
        should_release_logger_ = true;
    }
    else
    {
        logger_ = logger;
    }
#else
    logger_ = logger;
#endif
}
//------------------------------------------------------------------------------
Decoder::~Decoder()
{
    if (should_release_logger_)
    {
        delete logger_;
        logger_ = nullptr;
    }
}
//------------------------------------------------------------------------------
void Decoder::Log(const char* fmt, ...)
{
    if (!logger_)
        return;

    va_list args;
    va_start(args, fmt);
    logger_->Log(fmt, args);
    va_end(args);
}
//------------------------------------------------------------------------------