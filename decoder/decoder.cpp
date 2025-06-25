#include "decoder.hpp"
#include <cstdarg>

//------------------------------------------------------------------------------
Decoder::Decoder(LogFunc ui_logger)
{
    ui_logger_ = ui_logger;
}
//------------------------------------------------------------------------------
Decoder::~Decoder()
{

}
//------------------------------------------------------------------------------
void Decoder::Log(const char *fmt, ...)
{
    if (ui_logger_)
    {
        va_list args;
        va_start(args, fmt);
        ui_logger_(fmt, args);
        va_end(args);
    }
}