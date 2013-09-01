/**
* Project: VSXu: Realtime modular visual programming engine.
*
* This file is part of Vovoid VSXu.
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


#include "_configuration.h"
#include "vsx_param.h"
#include "vsx_module.h"
#include "vsx_math_3d.h"
#include <vsx_platform.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <vsx_platform.h>



#include "module_system_blocker.h"
#include "module_system_blocker_loading.h"
#include "module_system_clock.h"
#include "module_system_joystick.h"
#include "module_system_shutdown.h"
#include "module_system_time.h"
#include "module_system_tm_m.h"
#include "module_system_trig_sequencer.h"
#include "module_system_file_chooser.h"
#include "module_string_resource_to_string.h"



//******************************************************************************
//*** F A C T O R Y ************************************************************
//******************************************************************************

#ifndef _WIN32
#define __declspec(a)
#endif

extern "C" {
__declspec(dllexport) vsx_module* create_new_module(unsigned long module, void* args);
__declspec(dllexport) void destroy_module(vsx_module* m,unsigned long module);
__declspec(dllexport) unsigned long get_num_modules();
}


vsx_module* create_new_module(unsigned long module, void* args) {
  VSX_UNUSED(args);
  switch(module) {
    case 0: return (vsx_module*)(new module_system_shutdown);
    case 1: return (vsx_module*)(new module_system_time);
    case 2: return (vsx_module*)(new module_system_blocker);
    case 3: return (vsx_module*)(new module_system_clock);
    case 4: return (vsx_module*)(new module_system_trig_sequencer);
    case 5: return (vsx_module*)(new module_system_blocker_loading);
    case 6: return (vsx_module*)(new module_system_file_chooser);
    case 7: return (vsx_module*)(new module_string_resource_to_string);

#if PLATFORM == PLATFORM_LINUX
    case 8: return (vsx_module*)(new module_system_joystick);
    #ifdef VSXU_TM
    case 9: return (vsx_module*)(new module_system_tm_m);
    #endif
#endif
  }
  return 0;
}

void destroy_module(vsx_module* m,unsigned long module) {
  switch(module) {
    case 0: delete (module_system_shutdown*)m; break;
    case 1: delete (module_system_time*)m; break;
    case 2: delete (module_system_blocker*)m; break;
    case 3: delete (module_system_clock*)m; break;
    case 4: delete (module_system_trig_sequencer*)m; break;
    case 5: delete (module_system_blocker_loading*)m; break;
    case 6: delete (module_system_file_chooser*)m; break;
    case 7: delete (module_string_resource_to_string*)m; break;
#if PLATFORM == PLATFORM_LINUX
    case 8: delete (module_system_joystick*)m; break;
    #ifdef VSXU_TM
    case 9: delete (module_system_tm_m*)m; break;
    #endif
#endif
  }
}

unsigned long get_num_modules() 
{
  unsigned long n = 8;
  #if PLATFORM == PLATFORM_LINUX
    n++;
    #ifdef VSXU_TM
    n++;
    #endif
  #endif
  return n;
}  

