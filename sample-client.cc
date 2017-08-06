
#include <iostream>
#include <stdexcept>

#include <GL/gl.h>

#include "wayland-client-helper.hpp"
#include "wayland-egl-helper.hpp"

int main() {
  int process_result = -1;

  try {
    auto display = scoped(wl_display_connect(nullptr), wl_display_disconnect);

    auto compositor = global_bind<wl_compositor>(display, 1);
    auto shell      = global_bind<wl_shell>     (display, 1);

    auto surface = wl_compositor_create_surface(compositor);

    Egl egl(display);
    auto egl_surface = egl.add_surface(surface, 320, 240);

    eglMakeCurrent(egl.display, egl_surface, egl_surface, egl.context);

    auto shell_surface = wl_shell_get_shell_surface(shell, surface);
    wl_shell_surface_set_toplevel(shell_surface);

    do {
      glClearColor(0.0, 0.5, 0.5, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl.display, egl_surface);
    } while (-1 != wl_display_dispatch(display));
  }
  catch (std::exception& ex) {
  }

  return process_result;
}
