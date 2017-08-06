#ifndef INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7
#define INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7

#include <vector>
#include <tuple>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

class Egl {
private:
  struct window_and_surface {
    Egl* egl;
    wl_egl_window* window;
    EGLSurface surface;

    window_and_surface(Egl* egl, wl_surface* surface, int width, int height)
      : egl(egl),
	window(wl_egl_window_create(surface, width, height)),
	surface(eglCreateWindowSurface(egl->display, egl->config, window, nullptr))
    {
    }
    ~window_and_surface() {
      wl_egl_window_destroy(window);
      eglDestroySurface(egl->display, surface);
    }
  };

public:
  Egl(wl_display* display)
    : display(eglGetDisplay(display)),
      config(nullptr),
      context(nullptr)
  {
    eglInitialize(this->display, nullptr, nullptr);

    eglBindAPI(EGL_OPENGL_API);
    EGLint attributes[] = {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_NONE,
    };

    EGLint num_config = 0;
    eglChooseConfig(this->display, attributes, &this->config, 1, &num_config);

    if (this->config) {
      this->context = eglCreateContext(this->display, config, EGL_NO_CONTEXT, nullptr);
    }
  }
  virtual ~Egl() {
    for (auto item : this->window_and_surface_list) {
      delete item;
    }

    if (this->config) {
      if (this->context) {
	eglDestroyContext(this->display, this->context);
      }
      std::free(this->config);
    }
    eglTerminate(this->display);
  }

  Egl(Egl const&) = delete;
  Egl& operator = (Egl const&) = delete;

  auto add_surface(wl_surface* surface, int width, int height) {
    this->window_and_surface_list.push_back(new window_and_surface(this, surface, width, height));
    return this->window_and_surface_list.back()->surface;
  }

  auto window(wl_surface* surface, int width, int height) {
    return scoped(wl_egl_window_create(surface, width, height),
		  wl_egl_window_destroy);
  }
  auto surface(wl_egl_window* window) {
    return scoped(eglCreateWindowSurface(this->display, this->config, window, nullptr),
    		  [this](auto p) { eglDestroySurface(this->display, p); });
  }

public:
  EGLDisplay display;
  EGLConfig config;
  EGLContext context;
  std::vector<window_and_surface*> window_and_surface_list;
};

#endif/*INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7*/
