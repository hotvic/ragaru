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

/**> HEADER FILES <**/
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


#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

// some but not all of this is code from PhysicsFS: http://icculus.org/physfs/
//  the zlib license on physfs allows this cut-and-pasting.
static int locateOneElement(char *buf)
{
    char *ptr;
    char **rc;
    char **i;
    DIR *dirp;

    //if (PHYSFS_exists(buf))
    if (access(buf, F_OK) == 0)
        return(1);  /* quick rejection: exists in current case. */

    ptr = strrchr(buf, '/');  /* find entry at end of path. */
    if (ptr == NULL)
    {
        dirp = opendir(".");
        ptr = buf;
    } /* if */
    else
    {
        *ptr = '\0';
        dirp = opendir(buf);
        *ptr = '/';
        ptr++;  /* point past dirsep to entry itself. */
    } /* else */

    struct dirent *dent;
    while ((dent = readdir(dirp)) != NULL)
    {
        if (strcasecmp(dent->d_name, ptr) == 0)
        {
            strcpy(ptr, dent->d_name); /* found a match. Overwrite with this case. */
            closedir(dirp);
            return(1);
        } /* if */
    } /* for */

    /* no match at all... */
    closedir(dirp);
    return(0);
} /* locateOneElement */


static inline const char *getUserDirByUID(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL)
        return(pw->pw_dir);
    return(NULL);
} /* getUserDirByUID */


static inline const char *getPrefPath(void)
{
    static char *prefpath = NULL;
    if (prefpath == NULL)
    {
        const char *homedir = getenv("HOME");
        if (homedir == NULL)
            homedir = getUserDirByUID();
        if (homedir == NULL)
            homedir = ".";  // oh well.

        const char *PREFPATHNAME = ".lugaru";
        size_t len = strlen(homedir) + strlen(PREFPATHNAME) + 2;
        prefpath = new char[len];
        snprintf(prefpath, len, "%s/%s", homedir, PREFPATHNAME);
    }
    return(prefpath);
}

static int locateCorrectCase(char *buf, bool makedirs)
{
    int rc;
    char *ptr;
    char *prevptr;

    ptr = prevptr = buf;
    while (ptr = strchr(ptr + 1, '/'))
    {
        *ptr = '\0';  /* block this path section off */
        rc = locateOneElement(buf);
        if (!rc)
        {
            if (makedirs)  /* normal if we're writing; build dirs! */
                mkdir(buf, S_IRWXU);
            else
            {
                *ptr = '/'; /* restore path separator */
                return(-2);  /* missing element in path. */
            } /* else */
        } /* if */
        *ptr = '/'; /* restore path separator */
    } /* while */

    /* check final element... */
    return(locateOneElement(buf) ? 0 : -1);
}


static int locateCorrectFile(char *buf, const char *mode)
{
    if (*buf == '\0')
        return(0);  /* Uh...I guess that's failure. */

    assert((mode[0] == 'w') || (mode[0] == 'r'));

    bool iswriting = (mode[0] == 'w');
    const char *prefpath = getPrefPath();
    size_t len = strlen(buf) + strlen(prefpath) + 2;
    char *prefpathfile = (char *) alloca(len);
    snprintf(prefpathfile, len, "%s/%s", prefpath, buf);

    int rc = locateCorrectCase(prefpathfile, iswriting);  /* favor prefpath. */
    if ( (rc == 0) || ((rc == -1) && (iswriting)) ) // found or create?
        strcpy(buf, prefpathfile);
    else if ((rc < 0) && (!iswriting))  /* not writing? Try game dir... */
        rc = locateCorrectCase(buf, iswriting);

    return(rc);
} /* locateCorrectFile */
