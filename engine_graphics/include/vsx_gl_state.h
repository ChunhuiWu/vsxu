#ifndef VSX_GL_STATE_H
#define VSX_GL_STATE_H

// shadow state of OpenGL to avoid glGet* calls

#include "vsx_platform.h"
#include <_engine_graphics_dllimport.h>

#include "vsx_math_3d.h"

///////////////////////////////////////////////////////////////////////////////
// WARNING:
//   NO Range checking performed in this class.
//   Be careful!
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// face direction
#define VSX_GL_FRONT 0
#define VSX_GL_BACK 1

const int gl_face_direction[] =
{
  #ifndef VSX_NO_GL
  GL_FRONT,
  GL_BACK
  #endif
};

///////////////////////////////////////////////////////////////////////////////
// depth functions
#define VSX_GL_NEVER 0
#define VSX_GL_LESS 1
#define VSX_GL_EQUAL 2
#define VSX_GL_LEQUAL 3
#define VSX_GL_GREATER 4
#define VSX_GL_NOTEQUAL 5
#define VSX_GL_GEQUAL 6
#define VSX_GL_ALWAYS 7

const int gl_depth_functions[] =
{
  #ifndef VSX_NO_GL
  GL_NEVER,
  GL_LESS,
  GL_EQUAL,
  GL_LEQUAL,
  GL_GREATER,
  GL_NOTEQUAL,
  GL_GEQUAL,
  GL_ALWAYS
  #endif
};


///////////////////////////////////////////////////////////////////////////////
// material types
#define VSX_GL_AMBIENT 0
#define VSX_GL_DIFFUSE 1
#define VSX_GL_SPECULAR 2
#define VSX_GL_EMISSION 3
#define VSX_GL_SHININESS 4

const int gl_material_types[] =
{
  #ifndef VSX_NO_GL
  GL_AMBIENT,
  GL_DIFFUSE,
  GL_SPECULAR,
  GL_EMISSION,
  GL_SHININESS
  #endif
};

///////////////////////////////////////////////////////////////////////////////
// matrix modes
#define VSX_GL_PROJECTION_MATRIX 0
#define VSX_GL_MODELVIEW_MATRIX 1
#define VSX_GL_TEXTURE_MATRIX 2
const int gl_matrix_modes[] =
{
  #ifndef VSX_NO_GL
    GL_PROJECTION,
    GL_MODELVIEW,
    GL_TEXTURE
  #endif
};

///////////////////////////////////////////////////////////////////////////////
// blend modes

// source factors
#define VSX_GL_ZERO 0
#define VSX_GL_ONE 1
#define VSX_GL_SRC_COLOR 2
#define VSX_GL_ONE_MINUS_SRC_COLOR 3
#define VSX_GL_DST_COLOR 4
#define VSX_GL_ONE_MINUS_DST_COLOR 5
#define VSX_GL_SRC_ALPHA 6
#define VSX_GL_ONE_MINUS_SRC_ALPHA 7
#define VSX_GL_DST_ALPHA 8
#define VSX_GL_ONE_MINUS_DST_ALPHA 9
#define VSX_GL_CONSTANT_COLOR 10
#define VSX_GL_ONE_MINUS_CONSTANT_COLOR 11
#define VSX_GL_CONSTANT_ALPHA 12
#define VSX_GL_ONE_MINUS_CONSTANT_ALPHA 13
#define VSX_GL_SRC_ALPHA_SATURATE 14
#define VSX_GL_SRC1_COLOR 15
#define VSX_GL_ONE_MINUS_SRC1_COLOR 16
#define VSX_GL_SRC1_ALPHA 17
#define VSX_GL_ONE_MINUS_SRC1_ALPHA 18

const int gl_blend_factors[] =
{
  #ifndef VSX_NO_GL
  GL_ZERO,
  GL_ONE,
  GL_SRC_COLOR,
  GL_ONE_MINUS_SRC_COLOR,
  GL_DST_COLOR,
  GL_ONE_MINUS_DST_COLOR,
  GL_SRC_ALPHA,
  GL_ONE_MINUS_SRC_ALPHA,
  GL_DST_ALPHA,
  GL_ONE_MINUS_DST_ALPHA,
  GL_CONSTANT_COLOR,
  GL_ONE_MINUS_CONSTANT_COLOR,
  GL_CONSTANT_ALPHA,
  GL_ONE_MINUS_CONSTANT_ALPHA,
  GL_SRC_ALPHA_SATURATE,
  GL_SRC1_COLOR,
  GL_ONE_MINUS_SRC1_COLOR,
  GL_SRC1_ALPHA,
  GL_ONE_MINUS_SRC1_ALPHA
  #endif
};

///////////////////////////////////////////////////////////////////////////////
// polygon fill modes
#define VSX_GL_POINT 0
#define VSX_GL_LINE 1
#define VSX_GL_FILL 2

const int gl_polygon_fill[] =
{
  #ifndef VSX_NO_GL
  GL_POINT,
  GL_LINE,
  GL_FILL
  #endif
};

VSX_ENGINE_GRAPHICS_DLLIMPORT class vsx_gl_state
{
public:

  vsx_gl_state()
  {
    _material_init_values_default();
    _viewport_init_values_default();
    _matrix_init();
    _blend_mode_init_default();
    _depth_init();
    _line_width_init();
    _point_size_init();
    _framebuffer_binding_init();
  }

//***************************************************************************
//*** FRAMEBUFFER BINDING ***************************************************
//***************************************************************************
public:
  void framebuffer_bind(int id)
  {
    _framebuffer_binding = id;
    #ifndef VSX_NO_GL
//    vsx_printf("binding buffer %d\n", id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
    #endif
  }

  int framebuffer_bind_get()
  {
//    vsx_printf("getting buffer %d\n", _framebuffer_binding);
    return _framebuffer_binding;
  }

private:

  int _framebuffer_binding;

  inline void _framebuffer_binding_init()
  {
    _framebuffer_binding = 0;
  }


//***************************************************************************
//*** POINT SIZE ************************************************************
//***************************************************************************
public:
  inline void point_size_set(float n)
  {
    _point_size = n;
    #ifndef VSX_NO_GL
    glPointSize( _line_width );
    #endif
  }

  inline float point_size_get()
  {
    return _point_size;
  }

private:
  float _point_size;

  inline void _point_size_init()
  {
    _point_size = 1.0f;
  }



//***************************************************************************
//*** LINE WIDTH ************************************************************
//***************************************************************************
public:
  inline void line_width_set(float n)
  {
    _line_width = n;
    #ifndef VSX_NO_GL
    glLineWidth( _line_width );
    #endif
  }

  inline float line_width_get()
  {
    return _line_width;
  }

private:
  float _line_width;

  inline void _line_width_init()
  {
    _line_width = 1.0f;
  }



//***************************************************************************
//*** DEPTH BUFFER **********************************************************
//***************************************************************************
public:

  // depth mask
  void depth_mask_set(int n, bool debug = false)
  {
    (void)debug;
//    if (n == _depth_mask_enabled)
//    {
//      vsx_printf("depth mask already this\n");
//      return;
//    }
    _depth_mask_enabled = n;
    #ifndef VSX_NO_GL
    if (n)
    {
      glDepthMask( GL_TRUE );
    } else
    {
      glDepthMask( GL_FALSE );
    }
    #else
    vsx_printf("warning: using depth mask without GL\n");
    #endif
  }

  int depth_mask_get()
  {
    return _depth_mask_enabled;
  }

  // depth test
  void depth_test_set(int n, bool debug = false)
  {
    (void)debug;
//    if (n == _depth_test_enabled)
//      return;
    _depth_test_enabled = n;
    #ifndef VSX_NO_GL
    if (_depth_test_enabled)
    {
      glEnable(GL_DEPTH_TEST);
    } else
    {
      glDisable(GL_DEPTH_TEST);
    }
    #endif
  }

  int depth_test_get()
  {
    return _depth_test_enabled;
  }

  // depth function
  int depth_function_get()
  {
    return _depth_function;
  }

  void depth_function_set(int n)
  {
//    if ( n == _depth_function )
//      return;
    _depth_function = n;
    #ifndef VSX_NO_GL
    glDepthFunc( gl_depth_functions[_depth_function] );
    #endif
  }

private:
  int _depth_mask_enabled;
  int _depth_test_enabled;
  int _depth_function;

  void _depth_init()
  {
    _depth_mask_enabled = 0;
    _depth_test_enabled = 0;
    _depth_function = 1;
  }



//***************************************************************************
//*** FILL MODE *************************************************************
//***************************************************************************
public:
  void polygon_mode_set(int face, int mode)
  {
    if (_polygon_mode[face] == mode) return;
    _polygon_mode[face] = mode;
    #ifndef VSX_NO_GL
      glPolygonMode(
        gl_face_direction[face],
        gl_polygon_fill[mode]
      );
    #endif
  }

  int polygon_mode_get(int face)
  {
    return _polygon_mode[face];
  }

private:
  int _polygon_mode[2];

  void polygon_mode_init_default()
  {
    _polygon_mode[0] = 0;
    _polygon_mode[1] = 0;
  }

//***************************************************************************
//*** BLEND MODE ************************************************************
//***************************************************************************

public:
  inline int blend_get()
  {
    return _blend_enabled;
  }

  inline void blend_set(int n)
  {
//    if (n == _blend_enabled) return;
    _blend_enabled = n;
    #ifndef VSX_NO_GL
    if (_blend_enabled)
    {
      glEnable(GL_BLEND);
    } else
    {
      glDisable(GL_BLEND);
    }
    #endif
  }

  inline int blend_src_get()
  {
    return _blend_src;
  }

  inline int blend_dst_get()
  {
    return _blend_dst;
  }

  inline void blend_color_v(float* res)
  {
    memcpy( res, &_blend_color[0], sizeof(float)*4 );
  }

  inline void blend_color_set(float r, float g, float b, float a)
  {
    _blend_color[0] = r;
    _blend_color[1] = g;
    _blend_color[2] = b;
    _blend_color[3] = a;
    #ifndef VSX_NO_GL
    if (!GLEW_EXT_blend_color) return;
    glBlendColor(
      _blend_color[0],
      _blend_color[1],
      _blend_color[2],
      _blend_color[3]
    );
    #endif
  }

  inline void blend_func(int src, int dst)
  {
//    if (
//        _blend_src == src
//        &&
//        _blend_dst == dst
//        ) return;
    _blend_src = src;
    _blend_dst = dst;
    #ifndef VSX_NO_GL
    glBlendFunc
    (
      gl_blend_factors[src],
      gl_blend_factors[dst]
    );
    #endif
  }

private:

  int _blend_enabled;
  int _blend_src;
  int _blend_dst;
  float _blend_color[4];

  void _blend_mode_init_default()
  {
    _blend_enabled = 0;
    _blend_src = 0;
    _blend_dst = 0;
    _blend_color[0] = 0.0f;
    _blend_color[1] = 0.0f;
    _blend_color[2] = 0.0f;
    _blend_color[3] = 1.0f;
  }

//***************************************************************************
//*** MATERIAL **************************************************************
//***************************************************************************

public:
  // res must be a float[4]
  inline void material_set_fv(int face_direction, int type, float* res)
  {
    material_colors[type][face_direction][0] = res[0];
    material_colors[type][face_direction][1] = res[1];
    material_colors[type][face_direction][2] = res[2];
    material_colors[type][face_direction][3] = res[3];
#ifndef VSX_NO_GL
    glMaterialfv(
          gl_face_direction[face_direction],
          gl_material_types[type],
      material_colors[type][face_direction]
    );
#endif
  }

  // res must be a float[4]
  inline void material_get_fv(int face_direction, int type, float* res)
  {
    res[0] = material_colors[type][face_direction][0];
    res[1] = material_colors[type][face_direction][1];
    res[2] = material_colors[type][face_direction][2];
    res[3] = material_colors[type][face_direction][3];
  }

  inline void get_material_fv_all( float* res)
  {
    memcpy(res, &material_colors[0][0][0], sizeof(material_colors));
  }

  inline void material_set_fv_all(float* res)
  {
    memcpy(&material_colors[0][0][0], res, sizeof(material_colors));
    for (size_t mat = 0; mat < 5; mat++)
    {
      for (size_t face = 0; face < 2; face++)
      {
        material_set_fv(face, mat, material_colors[mat][face]);
      }
    }
  }

  inline void material_set_default()
  {
    _material_init_values_default();
    #ifndef VSX_NO_GL
      // ambient
      glMaterialfv(
        GL_FRONT,
        GL_AMBIENT,
        material_colors[VSX_GL_AMBIENT][VSX_GL_FRONT]
      );
      glMaterialfv(
        GL_BACK,
        GL_AMBIENT,
        material_colors[VSX_GL_AMBIENT][VSX_GL_BACK]
      );

      // diffuse
      glMaterialfv(
        GL_FRONT,
        GL_DIFFUSE,
        material_colors[VSX_GL_DIFFUSE][VSX_GL_FRONT]
      );
      glMaterialfv(
        GL_BACK,
        GL_DIFFUSE,
        material_colors[VSX_GL_DIFFUSE][VSX_GL_BACK]
      );

      // specular
      glMaterialfv(
        GL_FRONT,
        GL_SPECULAR,
        material_colors[VSX_GL_SPECULAR][VSX_GL_FRONT]
      );
      glMaterialfv(
        GL_BACK,
        GL_SPECULAR,
        material_colors[VSX_GL_SPECULAR][VSX_GL_BACK]
      );

      // emission
      glMaterialfv(
        GL_FRONT,
        GL_EMISSION,
        material_colors[VSX_GL_EMISSION][VSX_GL_FRONT]
      );
      glMaterialfv(
        GL_BACK,
        GL_EMISSION,
        material_colors[VSX_GL_EMISSION][VSX_GL_BACK]
      );

      // shininess
      glMaterialfv(
        GL_FRONT,
        GL_SHININESS,
        material_colors[VSX_GL_SHININESS][VSX_GL_FRONT]
      );
      glMaterialfv(
        GL_BACK,
        GL_SHININESS,
        material_colors[VSX_GL_SHININESS][VSX_GL_BACK]
      );
    #endif // ifndef VSX_NO_GL
  }

private:

  inline void _material_init_values_default()
  {
    // ambient materials
    material_colors[VSX_GL_AMBIENT][VSX_GL_FRONT][0] = 0.2f;
    material_colors[VSX_GL_AMBIENT][VSX_GL_FRONT][1] = 0.2f;
    material_colors[VSX_GL_AMBIENT][VSX_GL_FRONT][2] = 0.2f;
    material_colors[VSX_GL_AMBIENT][VSX_GL_FRONT][3] = 1.0f;

    material_colors[VSX_GL_AMBIENT][VSX_GL_BACK] [0] = 0.2f;
    material_colors[VSX_GL_AMBIENT][VSX_GL_BACK] [1] = 0.2f;
    material_colors[VSX_GL_AMBIENT][VSX_GL_BACK] [2] = 0.2f;
    material_colors[VSX_GL_AMBIENT][VSX_GL_BACK] [3] = 1.0f;

    // diffuse
    material_colors[VSX_GL_DIFFUSE][VSX_GL_FRONT][0] = 0.8f;
    material_colors[VSX_GL_DIFFUSE][VSX_GL_FRONT][1] = 0.8f;
    material_colors[VSX_GL_DIFFUSE][VSX_GL_FRONT][2] = 0.8f;
    material_colors[VSX_GL_DIFFUSE][VSX_GL_FRONT][3] = 1.0f;

    material_colors[VSX_GL_DIFFUSE][VSX_GL_BACK] [0] = 0.8f;
    material_colors[VSX_GL_DIFFUSE][VSX_GL_BACK] [1] = 0.8f;
    material_colors[VSX_GL_DIFFUSE][VSX_GL_BACK] [2] = 0.8f;
    material_colors[VSX_GL_DIFFUSE][VSX_GL_BACK] [3] = 1.0f;

    // specular
    material_colors[VSX_GL_SPECULAR][VSX_GL_FRONT][0] = 0.0f;
    material_colors[VSX_GL_SPECULAR][VSX_GL_FRONT][1] = 0.0f;
    material_colors[VSX_GL_SPECULAR][VSX_GL_FRONT][2] = 0.0f;
    material_colors[VSX_GL_SPECULAR][VSX_GL_FRONT][3] = 1.0f;

    material_colors[VSX_GL_SPECULAR][VSX_GL_BACK] [0] = 0.0f;
    material_colors[VSX_GL_SPECULAR][VSX_GL_BACK] [1] = 0.0f;
    material_colors[VSX_GL_SPECULAR][VSX_GL_BACK] [2] = 0.0f;
    material_colors[VSX_GL_SPECULAR][VSX_GL_BACK] [3] = 1.0f;

    // emission
    material_colors[VSX_GL_EMISSION][VSX_GL_FRONT][0] = 0.0f;
    material_colors[VSX_GL_EMISSION][VSX_GL_FRONT][1] = 0.0f;
    material_colors[VSX_GL_EMISSION][VSX_GL_FRONT][2] = 0.0f;
    material_colors[VSX_GL_EMISSION][VSX_GL_FRONT][3] = 1.0f;

    material_colors[VSX_GL_EMISSION][VSX_GL_BACK] [0] = 0.0f;
    material_colors[VSX_GL_EMISSION][VSX_GL_BACK] [1] = 0.0f;
    material_colors[VSX_GL_EMISSION][VSX_GL_BACK] [2] = 0.0f;
    material_colors[VSX_GL_EMISSION][VSX_GL_BACK] [3] = 1.0f;

    // shininess
    material_colors[VSX_GL_SHININESS][VSX_GL_FRONT][0] = 0.0f;
    material_colors[VSX_GL_SHININESS][VSX_GL_BACK] [0] = 0.0f;
  }

  float material_colors[5][2][4];

//***************************************************************************
//*** VIEWPORT **************************************************************
//***************************************************************************

public:

  inline int viewport_get_width()
  {
    return _viewport_size[2];
  }

  inline int viewport_get_height()
  {
    return _viewport_size[3];
  }

  // called from window change event
  inline void viewport_change
  (
    int x0,
    int y0,
    int w,
    int h
  )
  {
    _viewport_size[0] = x0;
    _viewport_size[1] = y0;
    _viewport_size[2] = w;
    _viewport_size[3] = h;
  }

  // substitute for glGetIntegerv(GL_VIEWPORT)
  inline void viewport_get(int* res)
  {
    res[0] = _viewport_size[0];
    res[1] = _viewport_size[1];
    res[2] = _viewport_size[2];
    res[3] = _viewport_size[3];
  }

  // substitute for glViewport()
  inline void viewport_set
  (
    const int x0,
    const int y0,
    const int w,
    const int h
  )
  {
    _viewport_size[0] = x0;
    _viewport_size[1] = y0;
    _viewport_size[2] = w;
    _viewport_size[3] = h;
#ifndef VSX_NO_GL
    glViewport(x0,y0,w,h);
#endif
  }

private:
  int	_viewport_size[4];

  inline void _viewport_init_values_default()
  {
    _viewport_size[0] = 0;
    _viewport_size[1] = 0;
    _viewport_size[2] = 0;
    _viewport_size[3] = 0;
  }

//***************************************************************************
//*** MATRIX OPS ************************************************************
//***************************************************************************

  vsx_matrix core_matrix[3];
  vsx_matrix matrix_stack[3][32];
  int i_matrix_stack_index[3];
  int i_matrix_mode;
  vsx_matrix m_temp;
  vsx_matrix m_temp_2;

  inline void _matrix_init()
  {
    i_matrix_stack_index[0] = 0;
    i_matrix_stack_index[1] = 0;
    i_matrix_stack_index[2] = 0;
    i_matrix_mode = 0;
  }

public:

  inline void matrix_load_identities()
  {
    core_matrix[0].load_identity();
    core_matrix[1].load_identity();
    core_matrix[2].load_identity();
    #ifndef VSX_NO_GL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    #endif
  }

  inline void matrix_get_v(int mode, float* res)
  {
    memcpy(res, &core_matrix[mode].m[0], sizeof(vsx_matrix) );
  }

  inline void matrix_mode(int new_mode)
  {
    i_matrix_mode = new_mode;
    #ifndef VSX_NO_GL
      glMatrixMode(gl_matrix_modes[i_matrix_mode]);
    #endif
  }

  inline void matrix_load_identity()
  {
    core_matrix[i_matrix_mode].load_identity();
    #ifndef VSX_NO_GL
      glLoadIdentity();
    #endif
  }

  // push and pop
  inline void matrix_push()
  {
    matrix_stack[i_matrix_mode][i_matrix_stack_index[i_matrix_mode]] = core_matrix[i_matrix_mode];
    i_matrix_stack_index[i_matrix_mode]++;
    #ifndef VSX_NO_GL
      glPushMatrix();
    #endif
  }

  inline void matrix_pop()
  {
    if (!i_matrix_stack_index)
      return;
    i_matrix_stack_index[i_matrix_mode]--;
    core_matrix[i_matrix_mode] = matrix_stack[i_matrix_mode][i_matrix_stack_index[i_matrix_mode]];
    #ifndef VSX_NO_GL
      glPopMatrix();
    #endif
  }



  inline void matrix_scale_f(float x, float y, float z, bool gl = true)
  {
    /*
    x 0 0 0
    0 y 0 0
    0 0 z 0
    0 0 0 1
    */
    m_temp.m[0] = x;     m_temp.m[4] = 0.0f;   m_temp.m[8] = 0.0f;   m_temp.m[12] = 0.0f;
    m_temp.m[1] = 0.0f;  m_temp.m[5] = y;      m_temp.m[9] = 0.0f;   m_temp.m[13] = 0.0f;
    m_temp.m[2] = 0.0f;  m_temp.m[6] = 0.0f;   m_temp.m[10] = z;     m_temp.m[14] = 0.0f;
    m_temp.m[3] = 0.0f;  m_temp.m[7] = 0.0f;   m_temp.m[11] = 0.0f;  m_temp.m[15] = 1.0f;

    memcpy(m_temp_2.m, core_matrix[i_matrix_mode].m, sizeof(vsx_matrix));
    core_matrix[i_matrix_mode].multiply( &m_temp, &m_temp_2);

    #ifndef VSX_NO_GL
    if (gl)
    {
      glLoadIdentity();
      glMultMatrixf(core_matrix[i_matrix_mode].m);
    }
    #else
    (void)gl;
    #endif
  }

  inline void matrix_translate_f(float x, float y, float z, bool gl = true)
  {
    /*
    1 0 0 x
    0 1 0 y
    0 0 1 z
    0 0 0 1
    */
    m_temp.m[0] = 1.0f;  m_temp.m[4] = 0.0f;   m_temp.m[8] = 0.0f;   m_temp.m[12] = x;
    m_temp.m[1] = 0.0f;  m_temp.m[5] = 1.0f;   m_temp.m[9] = 0.0f;   m_temp.m[13] = y;
    m_temp.m[2] = 0.0f;  m_temp.m[6] = 0.0f;   m_temp.m[10] = 1.0f;  m_temp.m[14] = z;
    m_temp.m[3] = 0.0f;  m_temp.m[7] = 0.0f;   m_temp.m[11] = 0.0f;  m_temp.m[15] = 1.0f;

    memcpy(m_temp_2.m, core_matrix[i_matrix_mode].m, sizeof(vsx_matrix));
    core_matrix[i_matrix_mode].multiply( &m_temp, &m_temp_2);

    #ifndef VSX_NO_GL
    if (gl)
    {
      glLoadIdentity();
      glMultMatrixf(core_matrix[i_matrix_mode].m);
    }
    #else
    (void)gl;
    #endif
  }

  inline void matrix_rotate_f(float angle, float x, float y, float z)
  {
    vsx_matrix m_rotate;
    /*
      Rotation matrix:
      xx(1-c)+c   xy(1-c)-zs  xz(1-c)+ys   0
      yx(1-c)+zs  yy(1-c)+c   yz(1-c)-xs   0
      xz(1-c)-ys  yz(1-c)+xs  zz(1-c)+c    0
      0           0           0            1

      c = cos(angle), s = sin(angle), and ||( x,y,z )|| = 1
    */
    float radians;
    radians = (angle * M_PI) / 180.0;
    float c = cos(radians);
    float s = sin(radians);
    float c1 = 1.0 - c;

    float xx = x*x;
    float yy = y*y;
    float zz = z*z;

    //normalize vector
    float length = sqrt(xx + yy + zz);
    if (length != 1.0f)
    {
      x = x / length;
      y = y / length;
      z = z / length;
    }

    m_rotate.m[0 ] = x*x*(c1)+c;
    m_rotate.m[4 ] = x*y*(c1)-z*s;
    m_rotate.m[8 ] = x*z*(c1)+y*s;

    m_rotate.m[1 ] = x*y*(c1)+z*s;
    m_rotate.m[5 ] = y*y*(c1)+c;
    m_rotate.m[9 ] = y*z*(c1)-x*s;

    m_rotate.m[2 ] = x*z*(c1)-y*s;
    m_rotate.m[6 ] = y*z*(c1)+x*s;
    m_rotate.m[10] = z*z*(c1)+c;

    memcpy(&m_temp.m[0], &core_matrix[i_matrix_mode].m[0], sizeof(vsx_matrix) );
    core_matrix[i_matrix_mode].multiply( &m_rotate, &m_temp);

    #ifndef VSX_NO_GL
      // TODO: use real OpenGL call here
      glLoadIdentity();
      glMultMatrixf(core_matrix[i_matrix_mode].m);
    #endif
  }

  inline void matrix_mult_f(float* res)
  {
    memcpy(&m_temp.m[0], res, sizeof(vsx_matrix) );
    memcpy(&m_temp_2.m[0], &core_matrix[i_matrix_mode].m[0], sizeof(vsx_matrix));
    core_matrix[i_matrix_mode].multiply( &m_temp, &m_temp_2 );
    #ifndef VSX_NO_GL
      glLoadIdentity();
      glMultMatrixf( core_matrix[i_matrix_mode].m );
    #endif

  }

  inline void matrix_ortho(double left, double right, double bottom, double top, double near, double far)
  {

    #define TX (right+left) / (right-left)
    #define TY (top + bottom) / (top - bottom)
    #define TZ (far + near) / (far - near)
    #define N0 2.0 / (right-left)
    #define N1 2.0 / (top-bottom)
    #define N2 -2.0 / (far-near)

    #define m m_temp.m

    // set up the matrix
    m[0] = N0;   m[4] = 0;    m[8]  =  0;    m[12]  = -TX;
    m[1] = 0;    m[5] = N1;   m[9]  =  0;    m[13]  = -TY;
    m[2] = 0;    m[6] = 0;    m[10] = N2;    m[14]  = -TZ;
    m[3] = 0;    m[7] = 0;    m[11] =  0;    m[15]  =  1;

    // undefine defines
    #undef m
    #undef TX
    #undef TY
    #undef TZ
    #undef N0
    #undef N1
    #undef N2

    // prepare for multiplication
    memcpy(&m_temp_2.m[0], &core_matrix[i_matrix_mode].m[0], sizeof(vsx_matrix));

    // multiply
    core_matrix[i_matrix_mode].multiply(&m_temp, &m_temp_2);

    // implement in OpenGL
    #ifndef VSX_NO_GL
      glMultMatrixf(m_temp.m);
    #endif
  }

  inline void matrix_glu_ortho_2d(
    float left,
    float right,
    float bottom,
    float top
  )
  {
    matrix_mode(VSX_GL_PROJECTION_MATRIX);
    matrix_load_identity();
    matrix_ortho( left, right, bottom, top, 0, 1 );
    matrix_mode(VSX_GL_MODELVIEW_MATRIX);
    matrix_load_identity();
  }

  inline void matrix_glu_perspective(
      double fovy,
      double aspect,
      double zNear,
      double zFar
  )
  {
    matrix_mode(VSX_GL_PROJECTION_MATRIX);
    matrix_load_identity();
    /*

    f is  cot( fovy / 2 )

    cot(x) = tan(HALF_PI - x)

    The generated matrix is
                 f
            ------------       0              0              0
               aspect


                0              f              0              0

                                          zFar+zNear    2*zFar*zNear
                0              0          ----------    ------------
                                          zNear-zFar     zNear-zFar

                0              0              -1             0
      */

#define N0 (zFar + zNear) / (zNear - zFar)
#define N1 (2.0 * zFar * zNear) / (zNear - zFar)
    double f = tan( HALF_PI - (fovy * (PI / 180.0f)) * 0.5 );
    double fdiva = f / aspect;
    #define m m_temp.m
    m[0] = fdiva;         m[4] = 0.0;      m[8] = 0.0;     m[12] = 0.0;
    m[1] = 0.0;          m[5] = f;         m[9] = 0.0;     m[13] = 0.0;
    m[2] = 0.0;          m[6] = 0.0;      m[10] = N0;      m[14] = N1;
    m[3] = 0.0;         m[7] = 0.0;     m[11] = -1.0;    m[15] = 0.0;

#undef m
#undef N0
#undef N1
  memcpy(&m_temp_2.m[0], &core_matrix[i_matrix_mode].m[0], sizeof(vsx_matrix));
  core_matrix[i_matrix_mode].multiply(&m_temp, &m_temp_2);
  #ifndef VSX_NO_GL
  glLoadIdentity();
  glMultMatrixf( core_matrix[i_matrix_mode].m );
#endif

    matrix_mode(VSX_GL_MODELVIEW_MATRIX);
  }

  inline void matrix_glu_lookat(
      float eyex, float eyey, float eyez,
      float centerx, float centery, float centerz,
      float upx, float upy, float upz
  )
  {
    float x[3], y[3], z[3];
    float mag;

    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
    }

    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
    }
    mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
    }
    #define M(row,col)  m_temp.m[col*4+row]
       M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
       M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
       M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
       M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
    #undef M

    memcpy(&m_temp_2.m[0], &core_matrix[i_matrix_mode].m[0], sizeof(vsx_matrix));
    core_matrix[i_matrix_mode].multiply(&m_temp,&m_temp_2);
    matrix_translate_f(-eyex, -eyey, -eyez, false);
    #ifndef VSX_NO_GL
      glLoadIdentity();
      glMultMatrixf(core_matrix[i_matrix_mode].m);
    #endif
  }

  static vsx_gl_state* get_instance()
  {
    static vsx_gl_state state;
    return &state;
  }


};



#endif
