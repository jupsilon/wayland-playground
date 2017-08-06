
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <tuple>

#include <cstring>

#include <GL/gl.h>

#include "wayland-client-helper.hpp"
#include "wayland-egl-helper.hpp"


int main() {
  int result = -1;

  try {
    std::cerr << "[enter] shell client" << std::endl;

    auto display = scoped(wl_display_connect(nullptr), wl_display_disconnect);

    auto compositor    = global_bind<wl_compositor>       (display, 4);
    auto seat          = global_bind<wl_seat>             (display, 1);
    auto output        = global_bind<wl_output>           (display, 1);
    auto desktop_shell = global_bind<weston_desktop_shell>(display, 1);

    auto surface       = scoped(wl_compositor_create_surface(compositor));

    wl_seat_listener seat_listener = {
      .capabilities = [](void*, wl_seat*, uint32_t capability) {
	std::cerr << "seat capability: " << capability << std::endl;
      },
      .name = [](void*, wl_seat*, char const* name) {
	std::cerr << "seat name: " << name << std::endl;
      },
    };
    wl_seat_add_listener(seat, &seat_listener, nullptr);
    wl_display_roundtrip(display);

    wl_output_listener output_listener = {
      .geometry = [](void* data, wl_output* output,
		     int x, int y, int physical_width, int physical_height,
		     int subpixel, char const* make, char const* model, int transform)
      {
	std::cerr << "geometry: ";
	std::cerr << x << ':';
	std::cerr << y << ':';
	std::cerr << physical_width << ':';
	std::cerr << physical_height << ':';
	std::cerr << subpixel << ':';
	std::cerr << make << ':';
	std::cerr << model << ':';
	std::cerr << transform << std::endl;
      },
      .mode = [](void* data, wl_output* output,
		 uint32_t flags, int width, int height, int refresh)
      {
	std::cerr << "mode: ";
	std::cerr << flags << ':';
	std::cerr << width << ':';
	std::cerr << height << ':';
	std::cerr << refresh << ':';
      },
      .done = [](void* data, wl_output* output) {
	std::cerr << "done." << std::endl;
      },
      .scale = [](void* data, wl_output* output, int factor) {
	std::cerr << "scale: ";
	std::cerr << factor << std::endl;
      },
    };
    std::cerr << output << std::endl;
    wl_output_add_listener(output, &output_listener, nullptr);
    wl_display_roundtrip(display);

    Egl egl(display);
    auto egl_window = egl.window(surface, 1920, 1080);
    auto egl_surface = egl.surface(egl_window);

    eglMakeCurrent(egl.display, egl_surface, egl_surface, egl.context);

    weston_desktop_shell_set_grab_surface(desktop_shell, surface);
    weston_desktop_shell_set_background(desktop_shell, output, surface);
    weston_desktop_shell_desktop_ready(desktop_shell);

    std::system("./sample-client &");
    std::system("weston-terminal &");

    {
      glClearColor(0.0, 0.0, 0.5, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(egl.display, egl_surface);
    }

    while (-1 != wl_display_dispatch(display));

    result = 0;

    std::cerr << "[leave] shell client" << std::endl;
  }
  catch (std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }

  return result;
}
