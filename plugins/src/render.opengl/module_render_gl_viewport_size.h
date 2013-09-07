class module_render_gl_viewport_size : public vsx_module
{
  GLint viewport[4];
  // in
  // out
  vsx_module_param_float* vx;
  vsx_module_param_float* vy;
  // internal

public:

  void module_info(vsx_module_info* info)
  {
    info->identifier = "system;viewport_size";
    info->description = "Gets current viewport size. Hook\nit to the screen or a texture buffer.";
    info->in_param_spec = "";
    info->out_param_spec = "vx:float,vy:float";
    info->component_class = "render";
    info->tunnel = true;
  }

  void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters)
  {
    VSX_UNUSED(in_parameters);
    loading_done = true;
    vx = (vsx_module_param_float*)out_parameters.create(VSX_MODULE_PARAM_ID_FLOAT,"vx");
    vx->set(0.0f);
    vy = (vsx_module_param_float*)out_parameters.create(VSX_MODULE_PARAM_ID_FLOAT,"vy");
    vy->set(0.0f);
  }

  void run()
  {
    vx->set( (float)engine->gl_state->get_viewport_width() );
    vy->set( (float)engine->gl_state->get_viewport_height() );
  }

};

