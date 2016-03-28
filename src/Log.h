/*
 * Copyright © 2015 - Victor A. Santos <victoraur.santos@gmail.com>
 *
 * This file is part of Ragaru.
 *
 * Ragaru is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>

namespace Ragaru
{
class Log
{
    bool consolelog, debug;
    std::ofstream output;

public:
    Log(bool console, bool dbg);
    void LOG(const char* fmt, ...);
    void DBG(const char* fmt, ...);
    void ERR(const char* fmt, ...);
};
}

#endif /* LOG_H */
