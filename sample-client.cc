
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
    auto window = attach_unique(wl_egl_window_create(surface.get(), 640, 480),
				wl_egl_window_destroy);

    egl_display_t egl_display(display.get());
    EGLint attr[] = {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_ALPHA_SIZE, 8,
      EGL_NONE,
    };
    auto egl_config = egl_display.choose_config(attr);
    egl_context_t egl_context(egl_display, egl_config, EGL_NO_CONTEXT, nullptr);
    egl_surface_t egl_surface(egl_display, egl_config, window.get(), nullptr);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    auto shell_surface = attach_unique(wl_shell_get_shell_surface(shell.get(), surface.get()));
    wl_shell_surface_set_toplevel(shell_surface.get());

    do {
      glClearColor(0.0, 0.0, 1.0, 0.5);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl_display, egl_surface);
    } while (-1 != wl_display_dispatch(display.get()));
  }
  catch (egl_error_t& ex) {
    std::cerr << "EGL error: " << eglGetError() << ' ' << ex.what() << std::endl;
  }
  catch (std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }

  return process_result;
}
