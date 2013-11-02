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

#include "vsx_command.h"
#include "vsx_command_list.h"
#include <time.h>


void vsx_command_list::clear(bool del) {
  if (del)
  {
    for (std::list <vsx_command_s*>::iterator it = commands.begin(); it != commands.end(); ++it) {
      (*it)->garbage_pointer->remove(*it);
      #ifdef VSXU_DEBUG
        printf("deleting command\n");
      #endif
      delete *it;
    }
  }
  commands.clear();
}

void vsx_command_list::load_from_file(vsx_string filename, bool parse, int type) {
#ifdef VSX_ENG_DLL
  if (!filesystem) {
    filesystem = new vsxf;
  }
  //printf("load_from_file VSX_ENG\n");
  vsxf_handle* fp;
  if ((fp = filesystem->f_open(filename.c_str(), "r")) == NULL)
  {
    #ifdef VSXU_DEBUG
    printf("error #1 opening file\n");
    #endif
    return;
  }
#else
  FILE* fp;
  if ((fp = fopen(filename.c_str(), "r")) == NULL)
  {
    #ifdef VSXU_DEBUG
    printf("error #2 opening file\n");
    #endif
    return;
  }
#endif
  char buf[65535];
  vsx_string line;
#ifdef VSX_ENG_DLL
  while (filesystem->f_gets((char*)&buf,65535,fp)) {
#else
  while (fgets((char*)buf,65535,fp)) {
#endif
    line = buf;
    //printf("load_from_file_run\n%s\n",line.c_str());
    if (line.size())
    {
    if (line[line.size()-1] == 0x0A) line.pop_back();
    if (line[line.size()-1] == 0x0D) line.pop_back();
    }
    //printf("load_from_file_run2\n%s\n",line.c_str());
    if (line != vsx_string("")) {
      if (parse) {
        add_raw(line);
        (*commands.back()).type = type;
      } else {
        vsx_command_s* t = new vsx_command_s;
        t->raw = line;
        t->type = type;
        commands.push_back(t);
      }
    }
  }
#ifdef VSX_ENG_DLL
  filesystem->f_close(fp);
#else
  fclose(fp);
#endif
}

void vsx_command_list::save_to_file(vsx_string filename) {
#ifndef VSX_NO_CLIENT
  #ifdef VSX_ENG_DLL
    if (!filesystem) {
      filesystem = new vsxf;
    }
    vsxf_handle* fp;
    if ((fp = filesystem->f_open(filename.c_str(), "w")) == NULL)
      return;

  #else
    FILE* fp;
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
      return;
  #endif
  for (std::list <vsx_command_s*>::iterator it = commands.begin(); it != commands.end(); ++it) {
    #ifdef VSX_ENG_DLL
      filesystem->f_puts(((*it)->raw+vsx_string("\n")).c_str(),fp);
    #else
      fputs(((*it)->raw+vsx_string("\n")).c_str(),fp);
    #endif
  }
  #ifdef VSX_ENG_DLL
    filesystem->f_close(fp);
  #else
    fclose(fp);
  #endif
#endif
}

void vsx_command_list::add(vsx_string cmd, vsx_string cmd_data) {
	if (!accept_commands) return;
	vsx_command_s* t = new vsx_command_s;
	t->cmd = cmd;
	t->cmd_data = cmd_data;
	t->parts.push_back(cmd);
	t->parts.push_back(cmd_data);
	t->raw = cmd+" "+cmd_data;
	commands.push_back(t);
}

void vsx_command_list::token_replace(vsx_string search, vsx_string replace) {
  for (std::list <vsx_command_s*>::iterator it = commands.begin(); it != commands.end(); ++it) {
    if ((*it)->parsed) {
      for (unsigned long i = 0; i < (*it)->parts.size(); ++i) {
        (*it)->parts[i] = str_replace(search, replace, (*it)->parts[i]);
      }
      (*it)->raw = str_replace(search, replace, (*it)->raw);
      //printf("token operations on parsed lists are not implemented yet! D.J. Aww knows this.");
    } else {
      (*it)->raw = str_replace(search, replace, (*it)->raw);
    }
  }
}

void vsx_command_list::parse() {
  for (std::list <vsx_command_s*>::iterator it = commands.begin(); it != commands.end(); ++it) {
    (*it)->parse();
  }
}

void vsx_command_list::adds(int tp, vsx_string titl,vsx_string cmd, vsx_string cmd_data) {
  if (!accept_commands) return;
  vsx_command_s* t = new vsx_command_s;
  t->type = tp;
  t->title = titl;
  t->cmd = cmd;
  t->cmd_data = cmd_data;
  t->parts.push_back(cmd);
  vsx_string deli = " ";
  vsx_avector<vsx_string> pp;
  explode(cmd_data,deli,pp);
  for (unsigned long i = 0; i < pp.size(); ++i)
  t->parts.push_back(pp[i]);
  //t->parts.push_back(cmd_data);

  t->raw = cmd+" "+cmd_data;

  commands.push_back(t);
}

void vsx_command_list::set_type(int new_type) {
	for (std::list <vsx_command_s*>::iterator it = commands.begin(); it != commands.end(); ++it)
  {
		(*it)->type = new_type;
	}
}

vsx_command_list::vsx_command_list()
  :
  filesystem(0),
  accept_commands(1)
{
  pthread_mutex_init(&mutex1, NULL);
}

