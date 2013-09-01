/**
* Project: VSXu Engine: Realtime modular visual programming engine.
*
* This file is part of Vovoid VSXu Engine.
*
* @author Jonatan Wallmander, Robert Wenzel, Vovoid Media Technologies AB Copyright (C) 2003-2013
* @see The GNU Lesser General Public License (LGPL)
*
* VSXu Engine is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU Lesser General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#ifndef VSX_TIMER_H
#define VSX_TIMER_H

//#ifndef WIN32
#include <time.h>
#include <sys/time.h>
//#endif

#ifdef _WIN32
#include <windows.h>
#endif

// get clock counter value (rdtsc)
static __inline__ double get_cc(void)
{
  unsigned a, d;
  asm volatile("rdtsc" : "=a" (a), "=d" (d));
  double f = 0.00000000029411764705882;
  return (double) (((uint64_t)a) | (((uint64_t)d) << 32)) *f;
}



class vsx_timer {
  double startt;
  double lastt;
  double dtimet;
#ifdef _WIN32  
  LONGLONG init_time;
#endif
public:
  
  void start() {
    startt = atime();
    lastt = startt;
  }
  
  double dtime() {
    double at = atime();
    dtimet = at-lastt;
    lastt = at;
    return dtimet;
  }
  
// normal time
double ntime() {
  return ((double)clock())/((double)CLOCKS_PER_SEC);
}

// accurate time
double atime() {
#ifdef _WIN32
  	LARGE_INTEGER freq, time;
  	QueryPerformanceFrequency(&freq);
  	QueryPerformanceCounter(&time);  	
    return (double)((double)(time.QuadPart-init_time) / (double)freq.QuadPart);; 
#else
    struct timeval now;
    gettimeofday(&now, 0);
    return (double)now.tv_sec+0.000001*(double)now.tv_usec;
#endif
}

vsx_timer() {
#ifdef _WIN32
 	LARGE_INTEGER time;
 	QueryPerformanceCounter(&time);
  init_time = time.QuadPart;
#endif
}

};


#endif
