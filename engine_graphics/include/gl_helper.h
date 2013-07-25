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


#ifndef VSX_GL_HELPER_H
#define VSX_GL_HELPER_H


#include <vsx_platform.h>
#include <_engine_graphics_dllimport.h>




VSX_ENGINE_GRAPHICS_DLLIMPORT void draw_box(vsx_vector pos, float width, float height);
VSX_ENGINE_GRAPHICS_DLLIMPORT void draw_box_c(vsx_vector pos, float width, float height);
VSX_ENGINE_GRAPHICS_DLLIMPORT void draw_box_tex(vsx_vector pos, float width, float height);
VSX_ENGINE_GRAPHICS_DLLIMPORT void draw_box_tex_c(vsx_vector pos, float width, float height);


#define draw_box_texf(pos_x, pos_y, pos_z, width, height) {\
\
  const GLshort squareTexcoords[] = {\
    0, 0,\
    1, 0,\
    0, 1,\
    1, 1\
  };\
\
  const GLfloat squareVertices[] = {\
    pos_x-width,   pos_y-width,\
    pos_x+width,   pos_y-width,\
    pos_x-width,   pos_y+height,\
    pos_x+width,   pos_y+height,\
  };\
\
  glVertexPointer(2, GL_FLOAT, 0, squareVertices);\
  glTexCoordPointer(2, GL_SHORT, 0, squareTexcoords);\
  glEnableClientState(GL_VERTEX_ARRAY);\
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);\
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);\
  glDisableClientState(GL_VERTEX_ARRAY);\
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);\
}






VSX_ENGINE_GRAPHICS_DLLIMPORT void draw_box_gradient(vsx_vector pos, float width, float height, vsx_color a, vsx_color b, vsx_color c, vsx_color d);
VSX_ENGINE_GRAPHICS_DLLIMPORT void draw_box_border(vsx_vector pos, vsx_vector size, float dragborder);
VSX_ENGINE_GRAPHICS_DLLIMPORT vsx_vector vsx_vec_viewport();



#endif
