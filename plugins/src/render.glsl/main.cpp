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
#include "vsx_gl_global.h"
#include "vsx_param.h"
#include "vsx_module.h"
#include "vsx_math_3d.h"
#include "default_shader.h"
#include <map>
#include "vsx_glsl.h"
#include <vsx_platform.h>


vsx_engine_environment* engine_environment = 0;

// stuff for loading pre-defined shaders from the file system
typedef struct {
  vsx_string name;
  vsx_string module_name;
} shader_info;

vsx_avector<shader_info> ext_shaders;

vsx_avector<unsigned long> init_run; // to keep track of the first time the module_info
// is queried, no need to link shaders in that step because the param_spec won't be read anyway.



void load_shader(vsx_glsl &shader, vsx_string filename)
{
  //printf("loading shader %s\n",filename.c_str());
  FILE* fp = fopen(filename.c_str(), "r");
  if (fp) {
    vsx_string s;
    vsx_string vert, frag;
    char line[4096];
    int state = 0;
    while (fgets(line,4096,fp))
    {
      if (vsx_string(line).find("*****") != -1) {
        ++state;
        //printf("*****INCREASING STATE\n");
      } else
      {
        if (!state) {
          vert = vert + vsx_string(line);
        } else {
          frag = frag + vsx_string(line);
        }
      }
    }
    shader.vertex_program = vert;
    shader.fragment_program = frag;
    fclose(fp);
  }
}



class vsx_module_glsl : public vsx_module
{
  // in
  vsx_module_param_render* render_in;
  vsx_module_param_int* reset_params;
  vsx_module_param_string* i_vertex_program;
  vsx_module_param_string* i_fragment_program;

  // out
  vsx_module_param_render* render_result;

  // internal
  float tax, tay, tbx, tby;
  std::map<vsx_string, vsx_module_param_abs*> shader_params;
  bool first;
  vsx_color cm;
  int cc;

public:
  vsx_glsl shader;
  long shader_source; // -1 for default, > 0 for filesystem loading of shaders



  vsx_module_glsl()
  {
    first = true;
    shader_source = 0;
  }

  void module_info(vsx_module_info* info)
  {
    if (shader_source == 0)
    {
      info->identifier = "renderers;shaders;glsl_loader";
    }
    else
    {
      info->identifier = "renderers;shaders;"+ext_shaders[shader_source-1].module_name;
    }

    info->in_param_spec = "render_in:render,vertex_program:string,fragment_program:string" + shader.get_param_spec();
    info->out_param_spec = "render_out:render";
    info->component_class = "render";
    info->tunnel = false;
  }

  void redeclare_in_params(vsx_module_param_list& in_parameters)
  {
    render_in = (vsx_module_param_render*)in_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_in");
    render_in->set(0);
    i_fragment_program = (vsx_module_param_string*)in_parameters.create(VSX_MODULE_PARAM_ID_STRING,"fragment_program");
    i_fragment_program->set(shader.fragment_program);
    i_vertex_program = (vsx_module_param_string*)in_parameters.create(VSX_MODULE_PARAM_ID_STRING,"vertex_program");
    i_vertex_program->set(shader.vertex_program.c_str());
    shader.declare_params(in_parameters);
  }

  void param_set_notify(const vsx_string& name)
  {
    if ((name == "vertex_program" || name == "fragment_program")) {
      shader.vertex_program = i_vertex_program->get();
      shader.fragment_program = i_fragment_program->get();
      message = shader.link();
      if (message.size() == 0)
      {
        redeclare_in = true;
        message = "module||ok";
      }
    }
  }

  void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters)
  {
    if (shader_source == 0)
    {
      shader.vertex_program = default_vert;
      shader.fragment_program = default_frag;
    } else {
      load_shader(shader,ext_shaders[shader_source-1].name);
    }
    printf("vert = %s\n",shader.vertex_program.c_str());
    printf("frag = %s\n",shader.fragment_program.c_str());

    vsx_string h = shader.link();

    loading_done = true;
    redeclare_in_params(in_parameters);
    render_result = (vsx_module_param_render*)out_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_out");
    render_result->set(0);
  }

  bool activate_offscreen()
  {
    #if PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_WINDOWS
      shader.begin();
      shader.set_uniforms();
    #else
      shader.set_uniforms();
      shader.begin();
    #endif
    return true;
  }

  void run()
  {
    render_result->set(0);
  }

  void deactivate_offscreen()
  {
    shader.end();
  }

  bool init()
  {
    #ifdef VSXU_OPENGL_ES_2_0
    if (!(GL_VERTEX_SHADER)) {
    #else
    if (!(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)) {
    #endif
      return false;
    }
    return true;
  }

};



//******************************************************************************
//*** F A C T O R Y ************************************************************
//******************************************************************************

#ifndef _WIN32
#define __declspec(a)
#endif

extern "C"
{
__declspec(dllexport) vsx_module* create_new_module(unsigned long module, void* args);
__declspec(dllexport) void destroy_module(vsx_module* m,unsigned long module);
__declspec(dllexport) unsigned long get_num_modules();
__declspec(dllexport) void set_environment_info(vsx_engine_environment* environment);
}


vsx_module* MOD_CM(unsigned long module, void* args)
{
  VSX_UNUSED(args);
  if (module)
  {
    vsx_module* v = (vsx_module*)(new vsx_module_glsl());
    ((vsx_module_glsl*)v)->shader_source = (long)module;
    return v;
  } else
  return (vsx_module*)(new vsx_module_glsl()); // module 0

  return 0;
}

void MOD_DM(vsx_module* m,unsigned long module)
{
  VSX_UNUSED(module);
  delete (vsx_module_glsl*)m;
}



unsigned long MOD_NM()
{
  #ifndef VSXU_OPENGL_ES
  glewInit();
  #endif

  // run once when vsxu starts
  init_run.push_back(0);
  std::list<vsx_string> i_shaders;
  vsx_string base_path;
  if (engine_environment)
  {
    base_path = engine_environment->engine_parameter[0];
    get_files_recursive(base_path+"render.glsl",&i_shaders,".glsl",".svn");
  }
  else
  {
    return 1;
  }

  unsigned long num_shaders = 0;
  for (std::list<vsx_string>::iterator it = i_shaders.begin(); it != i_shaders.end(); ++it) {
    vsx_string filename = *it;
    filename = str_replace(base_path, "", filename);

    shader_info info;
    info.name = *it;
    vsx_avector<vsx_string> parts;

    vsx_string deli = "/";
    explode(filename, deli, parts);
    vsx_avector<vsx_string> name_result;

    if (parts.size() > 1) {
      for (unsigned long i = 1; i < parts.size(); ++i) {
        name_result.push_back(parts[i]);
      }
      vsx_string deli_semi = ";";
      info.module_name = str_replace(".glsl","",implode(name_result, deli_semi));
      ext_shaders.push_back(info);
      ++num_shaders;
      init_run.push_back(0);
    }
  }
  return 1 + num_shaders;
}

void set_environment_info(vsx_engine_environment* environment)
{
  engine_environment = environment;
}
