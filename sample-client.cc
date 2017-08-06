
#include <iostream>
#include <stdexcept>

#include <GL/gl.h>

#include "wayland-client-helper.hpp"
#include "wayland-egl-helper.hpp"

int main() {
  int process_result = -1;

  try {
    auto display = attach_unique(wl_display_connect(nullptr), wl_display_disconnect);

    auto compositor = global_bind<wl_compositor>(display.get(), 1);
    auto shell      = global_bind<wl_shell>     (display.get(), 1);

    auto surface = attach_unique(wl_compositor_create_surface(compositor.get()));

    {
    egl_display_t egl_display(display.get());
    std::cerr << egl_display << std::endl;

    auto config = egl_display_choose_config(egl_display.get(),
					    utilities::array<EGLint>(EGL_RED_SIZE,   8,
								     EGL_GREEN_SIZE, 8,
								     EGL_BLUE_SIZE,  8,
								     // EGL_ALPHA_SIZE, 8,
								     EGL_NONE));
    dump(std::cerr, egl_display.get(), config);

    }

    Egl egl(display.get());
    auto egl_surface = egl.add_surface(surface.get(), 320, 240);

    eglMakeCurrent(egl.display, egl_surface, egl_surface, egl.context);

    auto shell_surface = attach_unique(wl_shell_get_shell_surface(shell.get(), surface.get()));
    wl_shell_surface_set_toplevel(shell_surface.get());

    do {
      glClearColor(0.0, 0.5, 0.5, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl.display, egl_surface);
    } while (-1 != wl_display_dispatch(display.get()));
  }
  catch (std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }

  return process_result;
}
