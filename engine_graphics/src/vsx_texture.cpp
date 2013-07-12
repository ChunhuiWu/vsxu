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


#include <vsxfst.h>
#include <vsx_gl_global.h>
#include <vsx_texture.h>
#ifndef VSX_TEXTURE_NO_GLPNG
  #include <vsxg.h>
  #include <stdlib.h>
  #ifdef _WIN32
    #include <pthread.h>
  #else
    #include <pthread.h>
  #endif
#endif

#include <vsx_gl_state.h>

std::map<vsx_string, vsx_texture_glist_holder> vsx_texture::t_glist;

vsx_texture::vsx_texture()
{
  gl_state = 0x0;
  pti_l = 0;
  valid = false;

  valid_fbo = false;
  frame_buffer_type = 0;
  frame_buffer_handle = 0;
  color_buffer_handle = 0;
  depth_buffer_handle = 0;
  depth_buffer_local = true;

  render_buffer_color_handle = 0;
  render_buffer_depth_handle = 0;
  frame_buffer_blit_handle = 0;

  capturing_to_buffer = false;
  is_glist_alias = false;
  texture_info = new vsx_texture_info;
  transform_obj = new vsx_transform_neutral;
  original_transform_obj = 1;
}

vsx_texture::vsx_texture(int id, int type)
{
  gl_state = 0x0;
  pti_l = 0;
  texture_info = new vsx_texture_info;
  texture_info->ogl_id = id;
  texture_info->ogl_type = type;
  transform_obj = new vsx_transform_neutral;
  valid = true;
  valid_fbo = false;
  capturing_to_buffer = false;
  is_glist_alias = false;
  #ifndef VSXU_OPENGL_ES
    glewInit();
  #endif
  depth_buffer_local = true;
}

vsx_texture::~vsx_texture()
{
  unload();
  if (!is_glist_alias) delete texture_info;
  if (original_transform_obj)
  delete transform_obj;
}













void vsx_texture::set_gl_state(void* n)
{
  gl_state = n;
}

void vsx_texture::init_opengl_texture_1d()
{
  GLuint tex_id;
  glGenTextures(1, &tex_id);
  texture_info->ogl_id = tex_id;
  texture_info->ogl_type = GL_TEXTURE_1D;
}


void vsx_texture::init_opengl_texture_2d()
{
  GLuint tex_id;
  glGenTextures(1, &tex_id);
  texture_info->ogl_id = tex_id;
  texture_info->ogl_type = GL_TEXTURE_2D;
}

void vsx_texture::init_opengl_texture_cubemap()
{
  GLuint tex_id;
  glGenTextures(1, &tex_id);
  texture_info->ogl_id = tex_id;
  texture_info->ogl_type = GL_TEXTURE_CUBE_MAP_EXT;
}

bool vsx_texture::has_buffer_support()
{
  bool fbo = GLEW_EXT_framebuffer_object;
  bool blit = GLEW_EXT_framebuffer_blit;
  if (!fbo) printf("vsx_texture Notice: EXT_framebuffer_object support is MISSING! This will mean limited functionality.\n\n");
  if (!blit) printf("vsx_texture Notice: EXT_framebuffer_blit support is MISSING! This will mean limited functionality.\n\n");
  return fbo && blit;
}


GLuint vsx_texture::get_depth_buffer_handle()
{
  return depth_buffer_handle;
}





void vsx_texture::init_render_buffer(
  int width,
  int height,
  bool float_texture,
  bool alpha,
  bool enable_multisample
)
{
  if (!gl_state) { vsx_printf("vsx_texture::init_feedback_buffer: vsx_texture gl_state not set!\n"); return; }
  prev_buf = 0;
  #ifndef VSXU_OPENGL_ES
    glewInit();
  #endif
  int i_width = width;
  int i_height = height;

  if ( !has_buffer_support() )
  {
    printf("vsx_texture error: No FBO available!\n");
    return;
  }

  // set the buffer type (for deinit and capturing)
  frame_buffer_type = VSX_TEXTURE_BUFFER_TYPE_RENDER_BUFFER;

  int prev_buf_l;
  prev_buf_l = ((vsx_gl_state*)gl_state)->framebuffer_bind_get();



  // create fbo for multi sampled content and attach depth and color buffers to it
  glGenFramebuffersEXT(1, &frame_buffer_handle);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_handle);





  // color render buffer
  glGenRenderbuffersEXT(1, &render_buffer_color_handle);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, render_buffer_color_handle);


  if(enable_multisample)
  {
    if (float_texture)
    {
      glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, 4, alpha?GL_RGBA16F_ARB:GL_RGB16F_ARB, width, height);
    }
    else
    {
      glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, 4, alpha?GL_RGBA8:GL_RGB8, width, height);
    }
  }
  else
  {
    if (float_texture)
    {
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, alpha?GL_RGBA16F_ARB:GL_RGB16F_ARB, width, height);
    }
    else
    {
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, alpha?GL_RGBA8:GL_RGB8, width, height);
    }
  }


  // depth render buffer
  glGenRenderbuffersEXT( 1, &render_buffer_depth_handle);
  glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, render_buffer_depth_handle);

  if(enable_multisample && GLEW_EXT_framebuffer_multisample)
  {
    glRenderbufferStorageMultisampleEXT(
      GL_RENDERBUFFER_EXT,
      4,
      GL_DEPTH_COMPONENT,
      width,
      height
    );
  }
  else
  {
    glRenderbufferStorageEXT(
      GL_RENDERBUFFER_EXT,
      GL_DEPTH_COMPONENT,
      width,
      height
    );
  }

  // attach the render buffers to the FBO handle
  glFramebufferRenderbufferEXT(
    GL_FRAMEBUFFER_EXT,
    GL_COLOR_ATTACHMENT0_EXT,
    GL_RENDERBUFFER_EXT,
    render_buffer_color_handle
  );
  glFramebufferRenderbufferEXT(
    GL_FRAMEBUFFER_EXT,
    GL_DEPTH_ATTACHMENT_EXT,
    GL_RENDERBUFFER_EXT,
    render_buffer_depth_handle
  );



  // Texture and FBO to blit into


  // create texture for blitting into
  glGenTextures(1, &color_buffer_handle);
  glBindTexture(GL_TEXTURE_2D, color_buffer_handle);
  if (float_texture)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, i_width, i_height, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  else
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, i_width, i_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,0);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set your texture parameters here if required ...



  // create a normal fbo and attach texture to it
  glGenFramebuffersEXT(1, &frame_buffer_blit_handle);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_blit_handle);

  // attach the texture to the FBO
  glFramebufferTexture2DEXT(
    GL_FRAMEBUFFER_EXT,
    GL_COLOR_ATTACHMENT0_EXT,
    GL_TEXTURE_2D,
    color_buffer_handle,
    0
  );

  // TODO: create and attach depth buffer also


  texture_info->ogl_id = color_buffer_handle;
  texture_info->ogl_type = GL_TEXTURE_2D;
  texture_info->size_x = width;
  texture_info->size_y = height;

  // restore eventual previous buffer
  ((vsx_gl_state*)gl_state)->framebuffer_bind(prev_buf_l);

  valid = true; // texture valid for binding
  valid_fbo = true; // fbo valid for capturing
}


void vsx_texture::deinit_render_buffer()
{
  glDeleteRenderbuffersEXT(1,&render_buffer_color_handle);
  glDeleteRenderbuffersEXT(1,&render_buffer_depth_handle);
  glDeleteTextures(1,&color_buffer_handle);
  glDeleteFramebuffersEXT(1, &frame_buffer_handle);
  glDeleteFramebuffersEXT(1, &frame_buffer_blit_handle);
  valid = false;
  valid_fbo = false;
  texture_info->ogl_id = 0;
  texture_info->ogl_type = 0;
}


void vsx_texture::reinit_render_buffer
(
  int width,
  int height,
  bool float_texture,
  bool alpha,
  bool multisample
)
{
  deinit_buffer();
  init_render_buffer
  (
    width,
    height,
    float_texture,
    alpha,
    multisample
  );
}










// init an offscreen color buffer, no depth
void vsx_texture::init_color_buffer
(
  int width, // width in pixels
  int height, // height in pixels
  bool float_texture, // use floating point channels (8-bit is default)
  bool alpha // support alpha channel or not
)
{
  if (!gl_state) { vsx_printf("vsx_texture::init_color_buffer: vsx_texture gl_state not set!\n"); return; }
  prev_buf = 0;
  #ifndef VSXU_OPENGL_ES
    glewInit();
  #endif
  int i_width = width;
  int i_height = height;

  if ( !has_buffer_support() )
  {
    printf("vsx_texture error: No FBO available!\n");
    return;
  }

  // set the buffer type (for deinit and capturing)
  frame_buffer_type = VSX_TEXTURE_BUFFER_TYPE_COLOR;

  // save the previous FBO (stack behaviour)
  int prev_buf_l;
  prev_buf_l = ((vsx_gl_state*)gl_state)->framebuffer_bind_get();
//  glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *)&prev_buf_l);

  GLuint texture_storage_type;

  if (float_texture)
  {
    texture_storage_type = alpha?GL_RGBA16F_ARB:GL_RGB16F_ARB;
  }
  else
  {
    texture_storage_type = alpha?GL_RGBA8:GL_RGB8;
  }

  //RGBA8 2D texture, 24 bit depth texture, 256x256
  glGenTextures(1, &color_buffer_handle);
  glBindTexture(GL_TEXTURE_2D, color_buffer_handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //NULL means reserve texture memory, but texels are undefined
  glTexImage2D(GL_TEXTURE_2D, 0, texture_storage_type, i_width, i_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

  //-------------------------
  glGenFramebuffersEXT(1, &frame_buffer_handle);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_handle);
  //Attach 2D texture to this FBO
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_buffer_handle, 0/*mipmap level*/);
  //-------------------------
  //Does the GPU support current FBO configuration?
  GLenum status;
  status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status)
  {
     case GL_FRAMEBUFFER_COMPLETE_EXT:
      texture_info->ogl_id = color_buffer_handle;
      texture_info->ogl_type = GL_TEXTURE_2D;
      texture_info->size_x = width;
      texture_info->size_y = height;
            valid = true; // valid for binding
      valid_fbo = true; // valid for capturing
      break;
    default:
    break;
  }
  ((vsx_gl_state*)gl_state)->framebuffer_bind(prev_buf_l);
}


void vsx_texture::deinit_color_buffer()
{
  //Delete resources
  glDeleteTextures(1, &color_buffer_handle);
  depth_buffer_handle = 0;
  depth_buffer_local = 0;
  //Bind 0, which means render to back buffer, as a result, fb is unbound
  glDeleteFramebuffersEXT(1, &frame_buffer_handle);

  valid = false;
  valid_fbo = false;
  texture_info->ogl_id = 0;
  texture_info->ogl_type = 0;
}

// run in stop/start or when changing resolution
void vsx_texture::reinit_color_buffer
(
  int width, // width in pixels
  int height, // height in pixels
  bool float_texture, // use floating point channels (8-bit is default)
  bool alpha // support alpha channel or not
)
{
  deinit_buffer();
  init_color_buffer
  (
    width,
    height,
    float_texture,
    alpha
  );

}








// init an offscreen color + depth buffer (as textures)
// See http://www.opengl.org/wiki/Framebuffer_Object_Examples
void vsx_texture::init_color_depth_buffer
(
  int width, // width in pixels
  int height, // height in pixels
  bool float_texture, // use floating point channels (8-bit is default)
  bool alpha, // support alpha channel or not
  GLuint existing_depth_texture_id
)
{
  if (!gl_state) { vsx_printf("vsx_texture::init_color_depth_buffer: vsx_texture gl_state not set!\n"); return; }
  prev_buf = 0;
  #ifndef VSXU_OPENGL_ES
    glewInit();
  #endif
  int i_width = width;
  int i_height = height;

  if ( !has_buffer_support() )
  {
    printf("vsx_texture error: No FBO available!\n");
    return;
  }

  // set the buffer type (for deinit and capturing)
  frame_buffer_type = VSX_TEXTURE_BUFFER_TYPE_COLOR_DEPTH;


  // save the previous FBO (stack behaviour)
  int prev_buf_l;
  prev_buf_l = ((vsx_gl_state*)gl_state)->framebuffer_bind_get();

  GLuint texture_storage_type;

  if (float_texture)
  {
    texture_storage_type = alpha?GL_RGBA16F_ARB:GL_RGB16F_ARB;
  }
  else
  {
    texture_storage_type = alpha?GL_RGBA8:GL_RGB8;
  }

  GLuint texture_type;
  texture_type = GL_TEXTURE_2D;



  // Generate Color Texture
  //RGBA8 2D texture, 24 bit depth texture, 256x256
  glGenTextures(1, &color_buffer_handle);
  glBindTexture(GL_TEXTURE_2D, color_buffer_handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //NULL means reserve texture memory, but texels are undefined
  glTexImage2D(GL_TEXTURE_2D, 0, texture_storage_type, i_width, i_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);







  // Generate Depth Texture
  if (existing_depth_texture_id != 0)
  {
    depth_buffer_handle = existing_depth_texture_id;
    depth_buffer_local = false;
  }
  else
  {
    glGenTextures(1, &depth_buffer_handle);
    glBindTexture(GL_TEXTURE_2D, depth_buffer_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, i_width, i_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    depth_buffer_local = true;
  }




  // GENERATE FBO
  //-------------------------
  glGenFramebuffersEXT(1, &frame_buffer_handle);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_handle);


  // Attach textures to the FBO


  //Attach 2D texture to this FBO
  glFramebufferTexture2DEXT(
    GL_FRAMEBUFFER_EXT,
    GL_COLOR_ATTACHMENT0_EXT,
    GL_TEXTURE_2D,
    color_buffer_handle,
    0/*mipmap level*/
  );
  //-------------------------
  //Attach depth texture to FBO
  glFramebufferTexture2DEXT(
    GL_FRAMEBUFFER_EXT,
    GL_DEPTH_ATTACHMENT_EXT,
    GL_TEXTURE_2D,
    depth_buffer_handle,
    0/*mipmap level*/
  );
  //-------------------------



  //Does the GPU support current FBO configuration?
  GLenum status;
  status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status)
  {
     case GL_FRAMEBUFFER_COMPLETE_EXT:
      texture_info->ogl_id = color_buffer_handle;
      texture_info->ogl_type = GL_TEXTURE_2D;
      texture_info->size_x = width;
      texture_info->size_y = height;
      valid = true; // valid for binding
      valid_fbo = true; // valid for capturing
      break;
    default:
    break;
  }
  ((vsx_gl_state*)gl_state)->framebuffer_bind(prev_buf_l);
}


void vsx_texture::deinit_color_depth_buffer()
{
  //Delete resources
  glDeleteTextures(1, &color_buffer_handle);
  if (depth_buffer_local)
  {
    glDeleteTextures(1, &depth_buffer_handle);
  }
  depth_buffer_handle = 0;
  depth_buffer_local = 0;
  //Bind 0, which means render to back buffer, as a result, fb is unbound
  if ( ((vsx_gl_state*)gl_state)->framebuffer_bind_get() == frame_buffer_handle )
    ((vsx_gl_state*)gl_state)->framebuffer_bind(0);
  glDeleteFramebuffersEXT(1, &frame_buffer_handle);

  valid = false;
  valid_fbo = false;
  texture_info->ogl_id = 0;
  texture_info->ogl_type = 0;
}


// run in stop/start or when changing resolution
VSX_TEXTURE_DLLIMPORT void vsx_texture::reinit_color_depth_buffer
(
  int width,
  int height,
  bool float_texture,
  bool alpha,
  GLuint existing_depth_texture_id
)
{
  deinit_buffer();
  init_color_depth_buffer
  (
    width,
    height,
    float_texture,
    alpha,
    existing_depth_texture_id
  );
}







void vsx_texture::deinit_buffer()
{
  // sanity checks
  if (!valid_fbo) return;

  if (!gl_state)
  {
    vsx_printf("vsx_texture::deinit_buffer: vsx_texture gl_state not set!\n");
    return;
  }

  if (frame_buffer_type == VSX_TEXTURE_BUFFER_TYPE_RENDER_BUFFER)
  {
    return deinit_render_buffer();
  }

  if (frame_buffer_type == VSX_TEXTURE_BUFFER_TYPE_COLOR)
  {
    deinit_color_buffer();
  }

  if (frame_buffer_type == VSX_TEXTURE_BUFFER_TYPE_COLOR_DEPTH)
  {
    deinit_color_depth_buffer();
  }

}






void vsx_texture::begin_capture_to_buffer()
{
  if (!gl_state) { vsx_printf("vsx_texture::begin_capture_to_buffer: vsx_texture gl_state not set!\n"); return; }
  if (!valid_fbo) return;
  if (capturing_to_buffer) return;

  prev_buf = ((vsx_gl_state*)gl_state)->framebuffer_bind_get();
  glPushAttrib(GL_ALL_ATTRIB_BITS );
  ((vsx_gl_state*)gl_state)->matrix_get_v( VSX_GL_PROJECTION_MATRIX, buffer_save_matrix[0].m );
  ((vsx_gl_state*)gl_state)->matrix_get_v( VSX_GL_MODELVIEW_MATRIX, buffer_save_matrix[1].m );
  ((vsx_gl_state*)gl_state)->matrix_get_v( VSX_GL_TEXTURE_MATRIX, buffer_save_matrix[2].m );


  ((vsx_gl_state*)gl_state)->matrix_mode( VSX_GL_PROJECTION_MATRIX );
  ((vsx_gl_state*)gl_state)->matrix_load_identity();
  ((vsx_gl_state*)gl_state)->matrix_mode( VSX_GL_MODELVIEW_MATRIX );
  ((vsx_gl_state*)gl_state)->matrix_load_identity();
  ((vsx_gl_state*)gl_state)->matrix_mode( VSX_GL_TEXTURE_MATRIX );
  ((vsx_gl_state*)gl_state)->matrix_load_identity();

  glEnable(GL_BLEND);

  ((vsx_gl_state*)gl_state)->framebuffer_bind(frame_buffer_handle);

  glViewport(0,0,(int)texture_info->size_x, (int)texture_info->size_y);

  capturing_to_buffer = true;
}


void vsx_texture::end_capture_to_buffer()
{
  if (!gl_state) { vsx_printf("vsx_texture::end_capture_to_buffer: vsx_texture gl_state not set!\n"); return; }
  if (!valid_fbo) return;
  if (capturing_to_buffer)
  {

    if (frame_buffer_type == VSX_TEXTURE_BUFFER_TYPE_RENDER_BUFFER)
    {
      glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, frame_buffer_handle);
      glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, frame_buffer_blit_handle);
      glBlitFramebufferEXT(
        0,
        0,
        (GLint)texture_info->size_x-1,
        (GLint)texture_info->size_y-1,
        0,
        0,
        (GLint)texture_info->size_x-1,
        (GLint)texture_info->size_y-1,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
      );
    }
    ((vsx_gl_state*)gl_state)->framebuffer_bind(prev_buf);
    ((vsx_gl_state*)gl_state)->matrix_mode( VSX_GL_PROJECTION_MATRIX );
    ((vsx_gl_state*)gl_state)->matrix_load_identity();
    ((vsx_gl_state*)gl_state)->matrix_mult_f( buffer_save_matrix[0].m );
    ((vsx_gl_state*)gl_state)->matrix_mode( VSX_GL_MODELVIEW_MATRIX );
    ((vsx_gl_state*)gl_state)->matrix_load_identity();
    ((vsx_gl_state*)gl_state)->matrix_mult_f( buffer_save_matrix[1].m );
    ((vsx_gl_state*)gl_state)->matrix_mode( VSX_GL_TEXTURE_MATRIX );
    ((vsx_gl_state*)gl_state)->matrix_load_identity();
    ((vsx_gl_state*)gl_state)->matrix_mult_f( buffer_save_matrix[2].m );

    glPopAttrib();
    capturing_to_buffer = false;
  }
}

void vsx_texture::unload_all_active()
{
  for (std::map<vsx_string, vsx_texture_glist_holder>::iterator it = t_glist.begin(); it != t_glist.end(); ++it)
  {
    glDeleteTextures(1,&((*it).second.texture_info.ogl_id));
  }
}

void vsx_texture::reinit_all_active()
{
  std::map<vsx_string, vsx_texture_glist_holder> temp_glist = t_glist;
  vsx_string tname;
  for (std::map<vsx_string, vsx_texture_glist_holder>::iterator it = temp_glist.begin(); it != temp_glist.end(); ++it)
  {
    if ((*it).second.texture_info.type == VSX_TEXTURE_INFO_TYPE_PNG)
    {
      tname = (*it).first;
      t_glist.erase(tname);
      load_png(tname);
    }
    if ((*it).second.texture_info.type == VSX_TEXTURE_INFO_TYPE_JPG)
    {
      tname = (*it).first;
      t_glist.erase(tname);
      load_jpeg(tname);
    }
  }
}


void vsx_texture::upload_ram_bitmap_1d( void* data, unsigned long size, bool mipmaps, int bpp, int bpp2 )
{
  GLboolean oldStatus = glIsEnabled(texture_info->ogl_type);

  glEnable(texture_info->ogl_type);
  glBindTexture(texture_info->ogl_type, texture_info->ogl_id);




  if (mipmaps)
  {
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
  } else
  {
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }

  // no compression
  if (bpp == GL_RGBA32F_ARB)
  {
    glTexImage1D(
      texture_info->ogl_type,  // opengl type
      0, // mipmap level
      GL_RGBA32F_ARB, // internal format
      size, // size
      0, // border
      bpp2, // source data format
      GL_FLOAT, // source data type
      data
    );
  } else
  {
    if (bpp == 3)
    {
      glTexImage1D(
        texture_info->ogl_type,  // opengl type
        0,  // mipmap level
        GL_COMPRESSED_RGB_ARB, // storage type
        size, // size x
        0,      // border 0 or 1
        bpp2,   // source data format
        GL_UNSIGNED_BYTE, // source data type
        data // pointer to data
      );
    }
    else
    {
      glTexImage1D(
        texture_info->ogl_type,  // opengl type
        0,  // mipmap level
        GL_COMPRESSED_RGBA_ARB, // storage type
        size, // size x
        0,      // border 0 or 1
        bpp2,   // source data format
        GL_UNSIGNED_BYTE, // source data type
        data // pointer to data
      );
    }
  }



  if(!oldStatus)
  {
    glDisable(texture_info->ogl_type);
  }
  valid = true;

}

void vsx_texture::upload_ram_bitmap_2d(vsx_bitmap* vbitmap,bool mipmaps, bool upside_down)
{
  upload_ram_bitmap_2d(vbitmap->data, vbitmap->size_x, vbitmap->size_y,mipmaps,vbitmap->bpp, vbitmap->bformat,upside_down);
}

void vsx_texture::upload_ram_bitmap_2d(void* data, unsigned long size_x, unsigned long size_y, bool mipmaps, int bpp, int bpp2, bool upside_down)
{
  // prepare data
  if (upside_down)
  {
    //printf("texture is upside down\n");
    if (bpp == GL_RGBA32F_ARB)
    {
      GLfloat* data2 = new GLfloat[size_x * size_y * 4];
      int dy = 0;
      int sxbpp = size_x*4;
      for (int y = size_y-1; y >= 0; --y) {
        for (unsigned long x = 0; x < size_x*4; ++x) {
          data2[dy*sxbpp + x] = ((GLfloat*)data)[y*sxbpp + x];
        }
        ++dy;
      }
      data = (GLfloat*)data2;
    }
    else
    {
      unsigned char* data2 = new unsigned char[(size_x) * (size_y) * (bpp)];
      int dy = 0;
      int sxbpp = size_x*bpp;
      for (int y = size_y-1; y >= 0; --y)
      {
        //printf("y: %d\n",y);
        int dysxbpp = dy*sxbpp;
        int ysxbpp = y * sxbpp;
        for (size_t x = 0; x < size_x*bpp; ++x)
        {
          data2[dysxbpp + x] = ((unsigned char*)data)[ysxbpp + x];
        }
        ++dy;
      }
      data = (unsigned long*)data2;
    }
  }






  GLboolean oldStatus = glIsEnabled(texture_info->ogl_type);

  glEnable(texture_info->ogl_type);
  glBindTexture(texture_info->ogl_type, texture_info->ogl_id);

  if (mipmaps)
  {
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    float rMaxAniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &rMaxAniso);
    glTexParameterf( texture_info->ogl_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, rMaxAniso);

  } else
  {
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }

  // no compression
  if (bpp == GL_RGBA32F_ARB)
  {
    glTexImage2D(texture_info->ogl_type, 0,bpp , size_x, size_y, 0, bpp2, GL_FLOAT, data);
  } else
  {
    if (bpp == 3)
    {
      glTexImage2D(
        texture_info->ogl_type,  // opengl type
        0,  // mipmap level
        GL_COMPRESSED_RGB_ARB, // storage type
        size_x, // size x
        size_y, // size y
        0,      // border 0 or 1
        bpp2,   // source data format
        GL_UNSIGNED_BYTE, // source data type
        data // pointer to data
      );
    }
    else
    {
      glTexImage2D(
        texture_info->ogl_type,  // opengl type
        0,  // mipmap level
        GL_COMPRESSED_RGBA_ARB, // storage type
        size_x, // size x
        size_y, // size y
        0,      // border 0 or 1
        bpp2,   // source data format
        GL_UNSIGNED_BYTE, // source data type
        data // pointer to data
      );
    }
    // original:
    //       glTexImage2D(texture_info->ogl_type, 0,bpp , size_x, size_y, 0, bpp2, GL_UNSIGNED_BYTE, data);
  }

  if (upside_down)
  {
    if (bpp == GL_RGBA32F_ARB)
    {
      delete[] (GLfloat*)data;
    } else
    {
      delete[] (unsigned long*)data;
    }
  }

  this->texture_info->size_x = size_x;
  this->texture_info->size_y = size_y;

  if(!oldStatus)
  {
    glDisable(texture_info->ogl_type);
  }
  valid = true;
}







void vsx_texture::upload_ram_bitmap_cube(void* data, unsigned long size_x, unsigned long size_y, bool mipmaps, int bpp, int bpp2, bool upside_down)
{
  if ( size_x / 6 != size_y )
  {
    vsx_printf("vsx_texture::upload_ram_bitmap_cube Error: not cubemap, should be aspect 6:1");
    return;
  }

  void* sides[6];

  // prepare data
  if (upside_down)
  {
    //printf("texture is upside down\n");
    if (bpp == GL_RGBA32F_ARB)
    {
      GLfloat* data2 = new GLfloat[size_x * size_y * 4];
      int dy = 0;
      int sxbpp = size_x*4;
      for (int y = size_y-1; y >= 0; --y) {
        for (unsigned long x = 0; x < size_x*4; ++x) {
          data2[dy*sxbpp + x] = ((GLfloat*)data)[y*sxbpp + x];
        }
        ++dy;
      }
      data = (GLfloat*)data2;


      // split cubemap into 6 individual bitmaps

      sides[0] = (void*)malloc( sizeof(GLfloat) * (size_y << 1) );
      sides[1] = (void*)malloc( sizeof(GLfloat) * (size_y << 1) );
      sides[2] = (void*)malloc( sizeof(GLfloat) * (size_y << 1) );
      sides[3] = (void*)malloc( sizeof(GLfloat) * (size_y << 1) );
      sides[4] = (void*)malloc( sizeof(GLfloat) * (size_y << 1) );
      sides[5] = (void*)malloc( sizeof(GLfloat) * (size_y << 1) );

      for (size_t side_offset = 0; side_offset < 6; side_offset++)
      {
        for (size_t y = 0; y < size_y; y++)
        {
          memcpy(
            // destination
            ((GLfloat*)sides[side_offset]) + y * size_y
            ,

            // souce
            (GLfloat*)&((GLfloat*)data)[ size_x * y ] // row offset
            +
            size_y * side_offset,            // horiz offset

            sizeof(GLfloat) * size_y // count
          );
        }
      }

    } else
    {
      unsigned char* data2 = new unsigned char[(size_x) * (size_y) * (bpp)];
      int dy = 0;
      int sxbpp = size_x*bpp;
      for (int y = size_y-1; y >= 0; --y)
      {
        //printf("y: %d\n",y);
        int dysxbpp = dy*sxbpp;
        int ysxbpp = y * sxbpp;
        for (size_t x = 0; x < size_x*bpp; ++x)
        {
          data2[dysxbpp + x] = ((unsigned char*)data)[ysxbpp + x];
        }
        ++dy;
      }
      data = (unsigned long*)data2;
    }

    // split cubemap into 6 individual bitmaps

    sides[0] = (void*)malloc( sizeof(uint32_t) * (size_y << 1) );
    sides[1] = (void*)malloc( sizeof(uint32_t) * (size_y << 1) );
    sides[2] = (void*)malloc( sizeof(uint32_t) * (size_y << 1) );
    sides[3] = (void*)malloc( sizeof(uint32_t) * (size_y << 1) );
    sides[4] = (void*)malloc( sizeof(uint32_t) * (size_y << 1) );
    sides[5] = (void*)malloc( sizeof(uint32_t) * (size_y << 1) );

    for (size_t side_offset = 0; side_offset < 6; side_offset++)
    {
      for (size_t y = 0; y < size_y; y++)
      {
        memcpy(
          // destination
          ((uint32_t*)sides[side_offset]) + y * size_y
          ,

          // souce
          (uint32_t*)&((uint32_t*)data)[ size_x * y ] // row offset
          +
          size_y * side_offset,            // horiz offset

          sizeof(uint32_t) * size_y // count
        );
      }
    }
  }





  GLboolean oldStatus = glIsEnabled(texture_info->ogl_type);

  glEnable(texture_info->ogl_type);
  glBindTexture(texture_info->ogl_type, texture_info->ogl_id);

  if (mipmaps)
  {
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    float rMaxAniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &rMaxAniso);
    glTexParameterf( texture_info->ogl_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, rMaxAniso);

  } else
  {
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texture_info->ogl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }


  // no compression
  if (bpp == GL_RGBA32F_ARB)
  {
    // TODO: upload!
    glTexImage2D(texture_info->ogl_type, 0,bpp , size_x, size_y, 0, bpp2, GL_FLOAT, data);
  } else
  {
    if (bpp == 3)
    {
      glTexImage2D(
        texture_info->ogl_type,  // opengl type
        0,  // mipmap level
        GL_COMPRESSED_RGB_ARB, // storage type
        size_x, // size x
        size_y, // size y
        0,      // border 0 or 1
        bpp2,   // source data format
        GL_UNSIGNED_BYTE, // source data type
        data // pointer to data
      );
    }
    else
    {
      glTexImage2D(
        texture_info->ogl_type,  // opengl type
        0,  // mipmap level
        GL_COMPRESSED_RGBA_ARB, // storage type
        size_x, // size x
        size_y, // size y
        0,      // border 0 or 1
        bpp2,   // source data format
        GL_UNSIGNED_BYTE, // source data type
        data // pointer to data
      );
    }
    // original:
    //       glTexImage2D(texture_info->ogl_type, 0,bpp , size_x, size_y, 0, bpp2, GL_UNSIGNED_BYTE, data);
  }

  if (upside_down)
  {
    if (bpp == GL_RGBA32F_ARB)
    {
      delete[] (GLfloat*)data;
    } else
    {
      delete[] (unsigned long*)data;
    }
  }

  this->texture_info->size_x = size_x;
  this->texture_info->size_y = size_y;

  if(!oldStatus)
  {
    glDisable(texture_info->ogl_type);
  }
  valid = true;
}










void vsx_texture::load_png(vsx_string fname, bool mipmaps, vsxf* filesystem)
{
  if (t_glist.find(fname) != t_glist.end())
  {
    if (!is_glist_alias)
    delete texture_info;
    texture_info = &t_glist[fname].texture_info;
    t_glist[fname].references++;
    this->name = fname;
    is_glist_alias = true;
    valid = true;
    return;
  }
  is_glist_alias = false;
  vsxf* i_filesystem = 0x0;
  //printf("processing png: %s\n",fname.c_str());
  if (filesystem == 0x0)
  {
    i_filesystem = new vsxf;
    filesystem = i_filesystem;
  }
  pngRawInfo* pp = new pngRawInfo;
  if (pngLoadRaw(fname.c_str(), pp, filesystem))
  {
    this->name = fname;
    init_opengl_texture_2d();
    if (pp->Components == 1)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,3,GL_RGB);
    if (pp->Components == 2)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,4,GL_RGBA);
    if (pp->Components == 3)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,pp->Components,GL_RGB);
    if (pp->Components == 4)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,pp->Components,GL_RGBA);

    free(pp->Data);

    if (pp->Palette)
    {
      free( pp->Palette );
    }

    texture_info->type = VSX_TEXTURE_INFO_TYPE_PNG; // png

    t_glist[fname].texture_info = *texture_info;
    delete texture_info;
    texture_info = &t_glist[fname].texture_info;
    t_glist[fname].references++;
    is_glist_alias = true;
  }
  delete pp;
  if (i_filesystem) delete i_filesystem;
}

// PNG THREAD STUFF
typedef struct
{
  pngRawInfo*       pp;
  int               thread_state;
  pthread_t					worker_t;
  pthread_attr_t		worker_t_attr;
  vsx_bitmap        bitmap;
  vsx_string       filename;
  bool              mipmaps;
} pti; // png thread info

// thread run by the load_png_thread
void* png_worker(void *ptr) {
  ((pti*)((vsx_texture*)ptr)->pti_l)->pp = new pngRawInfo;
  vsxf filesystem;
  if (pngLoadRaw(((pti*)((vsx_texture*)ptr)->pti_l)->filename.c_str(), ((pti*)((vsx_texture*)ptr)->pti_l)->pp,&filesystem))
  {
    ((pti*)((vsx_texture*)ptr)->pti_l)->thread_state = 2;
  }
  return 0;
}

// load a png but put the heavy processing in a thread
void vsx_texture::load_png_thread(vsx_string fname, bool mipmaps)
{
  is_glist_alias = false;
  if (t_glist.find(fname) != t_glist.end())
  {
    is_glist_alias = true;
    delete texture_info;
    texture_info = &t_glist[fname].texture_info;
    this->name = fname;
    valid = true;
    return;
  }
  is_glist_alias = false;
  if (pti_l) {
    if (((pti*)pti_l)->thread_state == 1) {
      long* aa;
      pthread_join(((pti*)pti_l)->worker_t, (void **)&aa);
    }
    free(((pti*)pti_l)->pp->Data);
    free(pti_l);
  }
  is_glist_alias = true;
  this->name = fname;
  valid = false;
  pti* pt = new pti;
  pt->filename = fname;
  pt->mipmaps = mipmaps;
  pthread_attr_init(&pt->worker_t_attr);
  pt->thread_state = 1;
  pti_l = (void*)pt;
  pthread_create(&(pt->worker_t), &(pt->worker_t_attr), &png_worker, (void*)this);
}

void vsx_texture::load_png_cubemap(vsx_string fname, bool mipmaps, vsxf* filesystem)
{
  if (t_glist.find(fname) != t_glist.end())
  {
    //printf("already found png: %s\n",fname.c_str());
    is_glist_alias = true;
    delete texture_info;
    texture_info = &t_glist[fname].texture_info;
    t_glist[fname].references++;
    valid = true;
    return;
  }
  is_glist_alias = true;
  vsxf* i_filesystem = 0x0;
  //printf("processing png: %s\n",fname.c_str());
  if (filesystem == 0x0)
  {
    i_filesystem = new vsxf;
    filesystem = i_filesystem;
  }
  pngRawInfo* pp = new pngRawInfo;
  if (pngLoadRaw(fname.c_str(), pp, filesystem))
  {
    this->name = fname;
    init_opengl_texture_cubemap();
    if (pp->Components == 1)
    upload_ram_bitmap_cube((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,3,GL_RGB);
    if (pp->Components == 2)
    upload_ram_bitmap_cube((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,4,GL_RGBA);
    if (pp->Components == 3)
    upload_ram_bitmap_cube((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,pp->Components,GL_RGB);
    if (pp->Components == 4)
    upload_ram_bitmap_cube((unsigned long*)(pp->Data),pp->Width,pp->Height,mipmaps,pp->Components,GL_RGBA);

    free(pp->Data);

    if (pp->Palette)
    {
      free( pp->Palette );
    }

    texture_info->type = VSX_TEXTURE_INFO_TYPE_PNG; // png


    t_glist[fname].texture_info = *texture_info;
    delete texture_info;
    texture_info = &t_glist[fname].texture_info;
    t_glist[fname].references++;
  }
  delete pp;
  if (i_filesystem) delete i_filesystem;
}












void vsx_texture::load_jpeg(vsx_string fname, bool mipmaps)
{
    CJPEGTest cj;
    vsx_string ret;
    vsxf filesystem;
    cj.LoadJPEG(fname,ret,&filesystem);
    upload_ram_bitmap_2d((unsigned long*)cj.m_pBuf, cj.GetResX(), cj.GetResY(), mipmaps, 3, GL_RGB);
    texture_info->type = VSX_TEXTURE_INFO_TYPE_JPG;
}


bool vsx_texture::bind()
{
  if (pti_l)
  if (((pti*)pti_l)->thread_state == 2)
  {
    if (texture_info->ogl_id != 0) unload();

    init_opengl_texture_2d();
    pngRawInfo* pp = (pngRawInfo*)(((pti*)pti_l)->pp);
    if (pp->Components == 1)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,((pti*)pti_l)->mipmaps,3,GL_RGB); else
    if (pp->Components == 2)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,((pti*)pti_l)->mipmaps,4,GL_RGBA); else
    if (pp->Components == 3)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,((pti*)pti_l)->mipmaps,pp->Components,GL_RGB); else
    if (pp->Components == 4)
    upload_ram_bitmap_2d((unsigned long*)(pp->Data),pp->Width,pp->Height,((pti*)pti_l)->mipmaps,pp->Components,GL_RGBA);
    free((((pti*)pti_l)->pp)->Data);
    texture_info->type = VSX_TEXTURE_INFO_TYPE_PNG; // png
    t_glist[name].texture_info = *texture_info;
    delete texture_info;
    texture_info = &t_glist[name].texture_info;
    t_glist[name].references++;
    pthread_join(((pti*)pti_l)->worker_t,0);
    valid = true;
    delete (pti*)pti_l;
    pti_l = 0;
  }
  if ( 0 == texture_info->ogl_id )
  {
    return false;
  }
  glEnable(texture_info->ogl_type);
  glBindTexture(texture_info->ogl_type,texture_info->ogl_id);
  return true;
}


void vsx_texture::_bind()
{
  if (texture_info->ogl_id == 0)
    return;
  glDisable(texture_info->ogl_type);
}

void vsx_texture::texcoord2f(float x, float y)
{
  #ifdef VSXU_OPENGL_ES
    printf("NO vsx_texture::texcoord2f support on OpenGL ES!\n");
  #else
    if (texture_info->ogl_type == GL_TEXTURE_RECTANGLE_EXT)
    {
      glTexCoord2i((GLuint)(x*texture_info->size_x),(GLuint)(y*texture_info->size_y));
      return;
    }
    glTexCoord2f(x,y);
  #endif
}

void vsx_texture::unload()
{
  if (texture_info->ogl_id == 0)
  {
    return;
  }

  if (!valid) return;

  if ( valid_fbo )
  {
    deinit_buffer();
    return;
  }

  // deal with reference counted textures
  if
  (
    name != ""
    &&
    t_glist.find(name) != t_glist.end()
  )
  {

    t_glist[name].references--;
    if (0 == t_glist[name].references)
    {
      // safe to delete this texture
      t_glist.erase( name );
      glDeleteTextures( 1, &(texture_info->ogl_id) );
      if (!is_glist_alias)
      {
        texture_info->ogl_id = 0;
        texture_info->ogl_type = 0;
      }
    }
    if (is_glist_alias)
    {
      texture_info = new vsx_texture_info;
    }
    valid = false;
    is_glist_alias = false;
    return;
  }

  glDeleteTextures(1,&(texture_info->ogl_id));
  texture_info->ogl_id = 0;
  texture_info->ogl_type = 0;
  valid = false;
}

