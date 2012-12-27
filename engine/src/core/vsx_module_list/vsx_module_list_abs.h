#ifndef VSX_MODULE_LIST_ABS_H
#define VSX_MODULE_LIST_ABS_H

// Module List Abstract Class
// Needs to be implemented per platform

class vsx_module_list_abs
{
private:
protected:
  // holder of dynamically linked object info, by string
  // the pointer should be typecast to vsx_module_dll_info*
  std::map< vsx_string, void* > module_dll_list;

  // module list (master)
  // holds module_infos harvested from the modules
  std::vector< vsx_module_info* > module_infos;

  // module list by string - holds all modules, including hidden modules
  std::map< vsx_string, vsx_module_info* > module_list;

  // module list intended for GUI consumption, doesn't contain hidden modules
  std::vector< vsx_module_info* > module_list_public;

public:
  // Init / Build Module List
  //   This method looks on disk for plugins, enumerates them
  //   and stores the results.
  //   Parameters:
  //     args:
  //       Can be used for config/things like:
  //         - Preferred driver (if dealing with hardware)
  //         - Config like OpenGL vsync on/off, FSAA on/off etc.
  //       Should only be used for stuff that has to stay the
  //       same during the whole instance of an engine. For everything
  //       else - use normal module parameters (maybe even hidden ones).
  //   Example use:
  //     build_module_list("sound.recording.type=fmod;opengl.vsync=1;");
  virtual void init(vsx_string args = "") = 0;

  // returns the modules
  virtual std::vector< vsx_module_info* > get_module_list( bool include_hidden = false) = 0;

  // Load Module By Name
  //   Used to load a vsxu module
  //   Parameters:
  //     name:
  //       Module identifier (i.e. path;to;module)
  //   Example use:
  //     load_module_by_name("math;oscillators;oscillator");
  virtual void* load_module_by_name(vsx_string name) = 0;

  // Unload Module
  virtual void* unload_module( vsx_module* module_pointer ) = 0;

  // Check presence of module
  virtual bool find( const vsx_string &module_name_to_look_for) = 0;
};

#endif
