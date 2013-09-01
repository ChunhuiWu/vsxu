class module_render_gl_line_width : public vsx_module
{
  // in
	vsx_module_param_render* render_in;
  vsx_module_param_float* width;
	// out
	vsx_module_param_render* render_out;
	// internal
  GLfloat prev_width;
public:
  void module_info(vsx_module_info* info) {
    info->identifier = "renderers;opengl_modifiers;gl_line_width";
    info->description = "";
    info->in_param_spec = "render_in:render,width:float";
    info->out_param_spec = "render_out:render";
    info->component_class = "render";
    loading_done = true;
    info->tunnel = true; // always run this
  }

	void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters) {
    render_in = (vsx_module_param_render*)in_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_in");
    width = (vsx_module_param_float*)in_parameters.create(VSX_MODULE_PARAM_ID_FLOAT,"width");
    width->set(1.0f);
    render_out = (vsx_module_param_render*)out_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_out");
    render_in->set(0);
    render_out->set(0);
  }
  bool activate_offscreen()
  {
    prev_width = engine->gl_state->line_width_get();
    engine->gl_state->line_width_set( width->get() );
    return true;
  }

	void deactivate_offscreen() {
    engine->gl_state->line_width_set( prev_width );
  }
};


class module_render_gl_texture_bind : public vsx_module
{
  // in
	vsx_module_param_render* render_in;
  vsx_module_param_texture* tex_in;
	// out
	vsx_module_param_render* render_out;
	// internal
	vsx_texture** t_tex;

public:
  void module_info(vsx_module_info* info) {
    info->identifier = "texture;opengl;texture_bind";
    info->description = "Uses OpenGL to bind a texture.\nAll connected to it (unless \n\
    it declares its own textures) \nwill use the texture \nprovided in the tex_in.";
    info->in_param_spec = "render_in:render,tex_in:texture";
    info->out_param_spec = "render_out:render";
    info->component_class = "texture";
    loading_done = true;
    info->tunnel = true;
  }

	void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters) {
    render_in = (vsx_module_param_render*)in_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_in");
    render_out = (vsx_module_param_render*)out_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_out");
    render_in->set(0);
    render_out->set(0);
    tex_in = (vsx_module_param_texture*)in_parameters.create(VSX_MODULE_PARAM_ID_TEXTURE,"tex_in");
    //tex_in->set(&i_tex);
  }

	bool activate_offscreen() {
    t_tex = tex_in->get_addr();
    if (t_tex)
    {
     	glMatrixMode(GL_TEXTURE);
     	glPushMatrix();
      vsx_transform_obj& texture_transform = *(*t_tex)->get_transform();
     	texture_transform();

      glMatrixMode(GL_MODELVIEW);


      (*t_tex)->bind();

    }
    return true;
  }

	void deactivate_offscreen() {
    if (t_tex) {
      (*t_tex)->_bind();
      glMatrixMode(GL_TEXTURE);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
  }
};


