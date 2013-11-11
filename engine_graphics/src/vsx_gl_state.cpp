#include <vsx_gl_global.h>
#include <vsx_gl_state.h>

vsx_gl_state* static_gl_state = 0x0;


vsx_gl_state* get_gl_state()
{
  if (!static_gl_state)
  {
    static_gl_state = new vsx_gl_state();
  }

  return static_gl_state;
}
