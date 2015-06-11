#include "Log.h"
#include <stdarg.h>


Lugaru::Log::Log(bool console, bool dbg)
{
    consolelog = console;
    debug = dbg;
}

void
Lugaru::Log::LOG(const char *fmt, ...)
{
    char buffer[255];

    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(buffer, fmt, argptr);
    va_end(argptr);

    if (consolelog)
        std::cout << buffer << std::endl;
    else {
        output.write(buffer, 255);
        output.put('\n');
    }
}

void
Lugaru::Log::DBG(const char *fmt, ...)
{
    if (!debug) return;

    char buffer[255];

    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(buffer, fmt, argptr);
    va_end(argptr);

    if (consolelog)
        std::cout << "DBG: " << buffer << std::endl;
    else {
        output.write("DBG: ", 6);
        output.write(buffer, 255);
        output.put('\n');
    }
}

void
Lugaru::Log::ERR(const char *fmt, ...)
{
    char buffer[255];

    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(buffer, fmt, argptr);
    va_end(argptr);

    std::cout << "ERR: " << buffer << std::endl;
    output.write("ERR: ", 6);
    output.write(buffer, 255);
    output.put('\n');
}
