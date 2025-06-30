#include "decoder.hpp"
#include "logger.hpp"
#include <cstdarg>

//------------------------------------------------------------------------------
Decoder::Decoder(Logger& logger)
    : logger_(logger)
{

}
//------------------------------------------------------------------------------
Decoder::~Decoder()
{

}
//------------------------------------------------------------------------------
void Decoder::Log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logger_.Log(fmt, args);
    va_end(args);
}
//------------------------------------------------------------------------------