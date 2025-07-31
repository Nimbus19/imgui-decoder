#include "decoder_apple.hpp"
#include <cstdio>

//------------------------------------------------------------------------------
DecoderApple::DecoderApple(LogFunc ui_logger)
    : Decoder(ui_logger)
{

}
//------------------------------------------------------------------------------
DecoderApple::~DecoderApple()
{

}
//------------------------------------------------------------------------------
bool DecoderApple::ReadMedia(const char* filePath, char* outMediaInfo, size_t infoSize)
{
    FILE* file = fopen(filePath, "r");
    if (file == nullptr)
    {
        sprintf(outMediaInfo, "Failed to open file: %s", filePath);
        return false;
    }

    fgets(outMediaInfo, (int)infoSize, file);

    fclose(file);
    return true;
}