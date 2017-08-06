
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <tuple>

#include <cstring>

#include <GL/gl.h>

#include "wayland-client-helper.hpp"
#include "wayland-egl-helper.hpp"


int main() {
  int process_result = -1;

  try {
    std::cerr << "[enter] shell client" << std::endl;

    auto display = attach_unique(wl_display_connect(nullptr), wl_display_disconnect);

    auto compositor    = global_bind<wl_compositor>       (display.get(), 4);
    auto seat          = global_bind<wl_seat>             (display.get(), 1);
    auto output        = global_bind<wl_output>           (display.get(), 1);
    auto desktop_shell = global_bind<weston_desktop_shell>(display.get(), 1);

    auto surface       = attach_unique(wl_compositor_create_surface(compositor.get()));
    auto window        = attach_unique(wl_egl_window_create(surface.get(), 1920, 1080),
				       wl_egl_window_destroy);

    egl_display_t egl_display(display.get());
    EGLint attr[] = {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_NONE,
    };

    auto egl_config = egl_display.choose_config(attr);
    egl_context_t egl_context(egl_display, egl_config, EGL_NO_CONTEXT, nullptr);
    egl_surface_t egl_surface(egl_display, egl_config, window.get(), nullptr);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    weston_desktop_shell_set_grab_surface(desktop_shell.get(), surface.get());
    weston_desktop_shell_set_background(desktop_shell.get(), output.get(), surface.get());
    weston_desktop_shell_desktop_ready(desktop_shell.get());

    std::system("weston-terminal &");
    std::system("./sample-client &");

    {
      glClearColor(0.0, 0.0, 0.5, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl_display, egl_surface);
    }

    while (-1 != wl_display_dispatch(display.get()));

    process_result = 0;

    std::cerr << "[leave] shell client" << std::endl;
  }
  catch (std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }

  return process_result;
}
