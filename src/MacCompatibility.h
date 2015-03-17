/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _MACCOMPATIBLITY_H_
#define _MACCOMPATIBLITY_H_

#if !PLATFORM_MACOSX

#include <stdio.h>
#include <float.h>
#include <math.h>

#ifndef __forceinline
#  ifdef __GNUC__
#    define __forceinline inline __attribute__((always_inline))
#  endif
#endif

typedef bool Boolean;


struct Point
{
	short v;
	short h;
};

typedef signed char SInt8;
typedef unsigned int UInt32;


#include "Random.h"


void CopyCStringToPascal( const char* src, unsigned char dst[256]);
void CopyPascalStringToC( const unsigned char* src, char* dst);


typedef struct AbsoluteTime
{
	unsigned long   hi;
	unsigned long   lo;
} AbsoluteTime; 

AbsoluteTime UpTime();		// NOTE: returns time since app started, not system start

typedef long Duration; 

enum
{
	durationMicrosecond             = -1,
	durationMillisecond             = 1,
	durationSecond                  = 1000,
	durationMinute                  = 1000 * 60,
	durationHour                    = 1000 * 60 * 60,
	durationDay                     = 1000 * 60 * 60 * 24,
	durationForever                 = 0x7FFFFFFF,
	durationImmediate               = 0,
}; 

Duration AbsoluteDeltaToDuration( AbsoluteTime& a, AbsoluteTime& b);

#include <stdint.h>


// fix file names to use '/' instead of ':'
char* ConvertFileName( const char* orgfilename, const char *mode = "rb" );

/*
inline float abs( float f)
{
if (f < 0)
return -f;
return f;
}

inline double abs( double f)
{
if (f < 0)
return -f;
return f;
}
*/
__forceinline long long longlongabs( long long f)
{
	if (f < 0)
		return -f;
	return f;
}
#endif
#endif


