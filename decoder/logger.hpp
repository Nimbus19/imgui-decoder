#pragma once

#include <cstdarg>
#include <vector>

class Logger
{
public:
    explicit Logger();
    virtual ~Logger();

    void Log(const char* fmt, ...);
    void Log(const char* fmt, va_list args);
    void AddOutput(char* buffer, size_t buffer_size);
    void RemoveOutput(char* buffer);
    void ClearOutputs();

protected:
    void ConsoleLog(const char* fmt, va_list args);

    std::vector<char*> outputs_;
    std::vector<size_t> output_sizes_;
};