#ifndef INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7
#define INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7

#include <memory>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <type_traits>
#include <iosfwd>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

#include "utilities/misc.hpp"

//////////////////////////////////////////////////////////////////////////////
struct egl_error_t : public std::runtime_error {
  egl_error_t(char const* where)
    : std::runtime_error(where)
  {
  }
};

//////////////////////////////////////////////////////////////////////////////
class egl_display_t :
  public std::unique_ptr<std::remove_pointer<EGLDisplay>::type, decltype (&eglTerminate)>
{
public:
  using base_type = std::unique_ptr<std::remove_pointer<EGLDisplay>::type, decltype (&eglTerminate)>;

public:
  egl_display_t(wl_display* display)
    : base_type(nullptr, eglTerminate)
  {
    auto egl_display = eglGetDisplay(display);
    if (EGL_NO_DISPLAY == egl_display) {
      throw egl_error_t(__PRETTY_FUNCTION__);
    }

    if (EGL_FALSE == eglInitialize(egl_display, &this->major, &this->minor)) {
      throw egl_error_t(__PRETTY_FUNCTION__);
    }
    base_type::reset(egl_display);
  }

  auto major_version() const { return this->major; }
  auto minor_version() const { return this->minor; }

  template <typename Ch>
  friend auto& operator << (std::basic_ostream<Ch>& output, egl_display_t const& display) {
    return output << "<EGLDisplay" << display.major << "." << display.minor << ">: " << display.get();
  }

private:
  EGLint major;
  EGLint minor;
};

//////////////////////////////////////////////////////////////////////////////

template <typename V>
inline auto egl_display_enum_config(EGLDisplay display, V&& attributes) {
  EGLint num_config = 0;
  if (EGL_FALSE == eglChooseConfig(display, &attributes.front(), nullptr, 0, &num_config)) {
    throw egl_error_t(__PRETTY_FUNCTION__);
  }

  std::vector<EGLConfig> configs(num_config);
  eglChooseConfig(display, &attributes.front(), &configs.front(), num_config, &num_config);
  return configs;
}

template <typename V>
inline auto egl_display_choose_config(EGLDisplay display, V&& attributes) {
  auto configs = egl_display_enum_config(display, std::move(attributes));
  if (configs.empty()) {
    throw std::runtime_error("no choosen config...");
  }
  return configs.front();
}

template <typename Ch>
inline auto& dump(std::basic_ostream<Ch>& output, EGLDisplay display, EGLConfig config) {
  constexpr std::tuple<EGLint, char const*> enum_and_label[] = {
#define MAKE_ENUM_AND_LABEL(x) std::tuple<EGLint, char const*>(x, #x)
    MAKE_ENUM_AND_LABEL(EGL_ALPHA_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_ALPHA_MASK_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_BIND_TO_TEXTURE_RGB),
    MAKE_ENUM_AND_LABEL(EGL_BIND_TO_TEXTURE_RGBA),
    MAKE_ENUM_AND_LABEL(EGL_BLUE_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_BUFFER_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_COLOR_BUFFER_TYPE),
    MAKE_ENUM_AND_LABEL(EGL_CONFIG_CAVEAT),
    MAKE_ENUM_AND_LABEL(EGL_CONFIG_ID),
    MAKE_ENUM_AND_LABEL(EGL_CONFORMANT),
    MAKE_ENUM_AND_LABEL(EGL_DEPTH_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_GREEN_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_LEVEL),
    MAKE_ENUM_AND_LABEL(EGL_LUMINANCE_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_MAX_PBUFFER_WIDTH),
    MAKE_ENUM_AND_LABEL(EGL_MAX_PBUFFER_HEIGHT),
    MAKE_ENUM_AND_LABEL(EGL_MAX_PBUFFER_PIXELS),
    MAKE_ENUM_AND_LABEL(EGL_MAX_SWAP_INTERVAL),
    MAKE_ENUM_AND_LABEL(EGL_MIN_SWAP_INTERVAL),
    MAKE_ENUM_AND_LABEL(EGL_NATIVE_RENDERABLE),
    MAKE_ENUM_AND_LABEL(EGL_NATIVE_VISUAL_ID),
    MAKE_ENUM_AND_LABEL(EGL_NATIVE_VISUAL_TYPE),
    MAKE_ENUM_AND_LABEL(EGL_RED_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_RENDERABLE_TYPE),
    MAKE_ENUM_AND_LABEL(EGL_SAMPLE_BUFFERS),
    MAKE_ENUM_AND_LABEL(EGL_SAMPLES),
    MAKE_ENUM_AND_LABEL(EGL_STENCIL_SIZE),
    MAKE_ENUM_AND_LABEL(EGL_SURFACE_TYPE),
    MAKE_ENUM_AND_LABEL(EGL_TRANSPARENT_TYPE),
    MAKE_ENUM_AND_LABEL(EGL_TRANSPARENT_RED_VALUE),
    MAKE_ENUM_AND_LABEL(EGL_TRANSPARENT_GREEN_VALUE),
    MAKE_ENUM_AND_LABEL(EGL_TRANSPARENT_BLUE_VALUE),
#undef MAKE_ENUM_AND_LABEL
  };

  for (auto const& item : enum_and_label) {
    output << std::get<1>(item) << '\t';

    EGLint value;
    if (eglGetConfigAttrib(display, config, std::get<0>(item), &value)) {
      output << value << std::endl;
    }
    else {
      output << '-' << std::endl;
    }
  }
  return output;
}

//////////////////////////////////////////////////////////////////////////////

inline auto egl_display_get_context_deleter(EGLDisplay display) {
  return [display](EGLContext context) {
    return eglDestroyContext(display, context);
  };
}

class egl_context_t :
  public std::unique_ptr<std::remove_pointer<EGLContext>::type,
			 decltype (egl_display_get_context_deleter(nullptr))>
{
public:
};

//////////////////////////////////////////////////////////////////////////////

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
    return attach_unique(wl_egl_window_create(surface, width, height),
			 wl_egl_window_destroy);
  }
  auto surface(wl_egl_window* window) {
    return attach_unique(eglCreateWindowSurface(this->display, this->config, window, nullptr),
			 [this](auto p) { eglDestroySurface(this->display, p); });
  }

public:
  EGLDisplay display;
  EGLConfig config;
  EGLContext context;
  std::vector<window_and_surface*> window_and_surface_list;
};

#endif/*INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7*/
