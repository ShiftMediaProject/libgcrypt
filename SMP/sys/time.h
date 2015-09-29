/*
 * MSVC sys/time.h compatability header.
 * Copyright (c) 2015 Matthew Oliver
 *
 * This file is part of Shift Media Project.
 *
 * Shift Media Project is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Shift Media Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the code; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _SMP_SYS_TIME_H_
#define _SMP_SYS_TIME_H_

#ifndef _MSC_VER
#   include_next <sys/time.h>
#else

#include <time.h>
#include <winsock2.h>

struct timezone
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

static __inline int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;
    static int tzflag;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - 116444736000000000Ui64) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    
    if (NULL != tzp)
    {
        if (!tzflag)
        {
          _tzset();
          tzflag++;
        }
        tzp->tz_minuteswest = _timezone / 60;
        tzp->tz_dsttime = _daylight;
    }
    return 0;
}

#endif /* _MSC_VER */

#endif /* _SMP_SYS_TIME_H_ */