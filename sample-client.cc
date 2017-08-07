
#include <iostream>
#include <stdexcept>

#include <GL/gl.h>

#include "wayland-client-helper.hpp"
#include "wayland-egl-helper.hpp"

int main() {
  int process_result = -1;

  try {
    auto display = attach_unique(wl_display_connect(nullptr), wl_display_disconnect);

    auto compositor    = global_bind<wl_compositor>   (display.get(), 1);
    auto subcompositor = global_bind<wl_subcompositor>(display.get(), 1);
    auto shell         = global_bind<wl_shell>        (display.get(), 1);

    egl_display_t egl_display(display.get());

    auto surface1 = attach_unique(wl_compositor_create_surface(compositor.get()));
    auto window1 = attach_unique(wl_egl_window_create(surface1.get(), 640, 480),
				 wl_egl_window_destroy);

    EGLint attr[] = {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_ALPHA_SIZE, 8,
      EGL_NONE,
    };
    auto egl_config1 = egl_display.choose_config(attr);
    egl_context_t egl_context1(egl_display, egl_config1, EGL_NO_CONTEXT, nullptr);
    egl_surface_t egl_surface1(egl_display, egl_config1, window1.get(), nullptr);

    auto shell_surface = attach_unique(wl_shell_get_shell_surface(shell.get(), surface1.get()));
    wl_shell_surface_set_toplevel(shell_surface.get());

    auto surface2 = attach_unique(wl_compositor_create_surface(compositor.get()));
    auto window2 = attach_unique(wl_egl_window_create(surface2.get(), 320, 240),
				 wl_egl_window_destroy);
    egl_surface_t egl_surface2(egl_display, egl_config1, window2.get(), nullptr);


    #if 1
    auto subsurface =  attach_unique(wl_subcompositor_get_subsurface(subcompositor.get(),
								     surface2.get(),
								     surface1.get()),
				     wl_subsurface_destroy);
    wl_subsurface_set_position(subsurface.get(), 16, 16);
    wl_subsurface_place_above(subsurface.get(), surface1.get());
    //wl_subsurface_set_sync(subsurface.get());
    #endif

    #if 0
    auto shell_surface2 = attach_unique(wl_shell_get_shell_surface(shell.get(), surface2.get()));
    wl_shell_surface_set_toplevel(shell_surface2.get());
    #endif 

    do {
      eglMakeCurrent(egl_display, egl_surface1, egl_surface1, egl_context1);
      glClearColor(0.0, 0.0, 1.0, 0.5);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl_display, egl_surface1);

      eglMakeCurrent(egl_display, egl_surface2, egl_surface2, egl_context1);
      glClearColor(0.0, 1.0, 1.0, 0.4);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl_display, egl_surface2);
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
