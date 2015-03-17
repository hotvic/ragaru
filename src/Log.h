#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>


namespace Lugaru
{
    class Log {
        bool consolelog, debug;
        std::ofstream output;

        public:
            Log(bool console, bool dbg);
            void LOG(const char *fmt, ...);
            void DBG(const char *fmt, ...);
    };
}

extern Lugaru::Log *LOG;

#endif /* LOG_H */
