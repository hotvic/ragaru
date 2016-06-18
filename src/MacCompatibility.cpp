/*
 * Copyright © 2015 - Victor A. Santos <victoraur.santos@gmail.com>
 * Copyright © 2003, 2010 - Wolfire Games
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

#include "MacCompatibility.h"

#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>

static int QueryPerformanceFrequency(int64_t *liptr)
{
    assert(sizeof (int64_t) == 8);
    *liptr = 1000;
    return(1);
}

static int QueryPerformanceCounter(int64_t *liptr)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *liptr = ( (((int64_t) tv.tv_sec) * 1000) +
               (((int64_t) tv.tv_usec) / 1000) );
    return(1);
}

class AppTime
{
public:
	AppTime()
	{
		counterRate = 1;
		baseCounter = 0;
		QueryPerformanceFrequency( (int64_t*)&counterRate);
		QueryPerformanceCounter( (int64_t*)&baseCounter);
	}
	int64_t counterRate;		// LARGE_INTEGER type has no math functions so use int64
	int64_t baseCounter;
};
static AppTime g_appTime;


void CopyCStringToPascal( const char* src, unsigned char dst[256])
{
	int len = strlen( src);
	dst[0] = len;
	memcpy( dst + 1, src, len);
}


void CopyPascalStringToC( const unsigned char* src, char* dst)
{
	int len = src[ 0];
	memcpy( dst, src + 1, len);
	dst[ len] = 0;
}


AbsoluteTime UpTime()
{
	int64_t counter;
	QueryPerformanceCounter( (int64_t*)&counter);

	counter -= g_appTime.baseCounter;

	AbsoluteTime time;
	time.lo = (unsigned long)counter;
	time.hi = (unsigned long)(counter >> 32);
	return time;
}


Duration AbsoluteDeltaToDuration( AbsoluteTime& a, AbsoluteTime& b)
{
	int64_t value = a.hi;
	value <<= 32;
	value |= a.lo;
	int64_t value2 = b.hi;
	value2 <<= 32;
	value2 |= b.lo;
	value -= value2;

	if (value <= 0)
		return durationImmediate;

	int64_t frac = value % g_appTime.counterRate;
	value /= g_appTime.counterRate;

	Duration time;

	if (value == 0)
	{
		frac *= -1000000;
		frac /= g_appTime.counterRate;
		time = (Duration)frac;
	}
	else
	{
		frac *= 1000;
		frac /= g_appTime.counterRate;
		value *= 1000;
		value += frac;
		time = (Duration)value;
	}

	return time;
}
