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


//////////////////////////////////////////////////////////////////////////////
struct egl_error_t : public std::runtime_error {
  egl_error_t(char const* where)
    : std::runtime_error(where)
  {
  }
};

//////////////////////////////////////////////////////////////////////////////
class egl_display_t
{
  egl_display_t(egl_display_t const&) = delete;
  egl_display_t& operator = (egl_display_t const&) = delete;

public:
  egl_display_t(wl_display* display)
    : egl_display(eglGetDisplay(display))
  {
    if (EGL_NO_DISPLAY == this->egl_display ||
	EGL_FALSE == eglInitialize(this->egl_display, &this->major, &this->minor))
    {
      throw egl_error_t(__PRETTY_FUNCTION__);
    }
  }
  ~egl_display_t() {
    eglTerminate(this->egl_display);
  }

  operator EGLDisplay() const { return this->egl_display; }
  auto major_version()  const { return this->major; }
  auto minor_version()  const { return this->minor; }

  template <typename Ch>
  friend auto& operator << (std::basic_ostream<Ch>& output, egl_display_t const& display) {
    output << "<EGLDisplay." << display.major << "." << display.minor << ">: ";
    output << display.egl_display;
    return output;
  }

  inline auto enum_config(EGLint const* attributes) {
    EGLint num_config = 0;
    auto ret = eglChooseConfig(this->egl_display, attributes, nullptr, 0, &num_config);
    if (EGL_FALSE == ret) {
      throw egl_error_t(__PRETTY_FUNCTION__);
    }

    std::vector<EGLConfig> configs(num_config);
    eglChooseConfig(this->egl_display, attributes, &configs.front(), num_config, &num_config);
    return configs;
  }

  inline auto choose_config(EGLint const* attributes) {
    auto configs = this->enum_config(attributes);
    if (configs.empty()) {
      throw std::runtime_error("no choosen config...");
    }
    return configs.front();
  }

  template <typename Ch>
  inline std::basic_ostream<Ch>& dump(std::basic_ostream<Ch>& output, EGLConfig config);

private:
  EGLDisplay egl_display;
  EGLint major;
  EGLint minor;
};

//////////////////////////////////////////////////////////////////////////////

class egl_context_t {
  egl_context_t(egl_context_t const&) = delete;
  egl_context_t& operator = (egl_context_t const&) = delete;

public:
  egl_context_t(EGLDisplay display, EGLConfig config, EGLContext shared, EGLint const* attr)
    : egl_context(eglCreateContext(display, config, shared, attr)),
      egl_display(display)
  {
    if (EGL_NO_CONTEXT == this->egl_context) {
      throw egl_error_t(__PRETTY_FUNCTION__);
    }
  }
  ~egl_context_t()
  {
    eglDestroyContext(this->egl_display, this->egl_context);
  }

public:
  operator EGLContext() const { return this->egl_context; }

private:
  EGLContext egl_context;
  EGLDisplay egl_display;
};

//////////////////////////////////////////////////////////////////////////////

class egl_surface_t {
  egl_surface_t(egl_surface_t const&) = delete;
  egl_surface_t& operator = (egl_context_t const&) = delete;

public:
  egl_surface_t(EGLDisplay display, EGLConfig config, wl_egl_window* window, EGLint const* attr)
    : egl_surface(eglCreateWindowSurface(display, config, window, attr)),
      egl_display(display)
  {
    if (EGL_NO_SURFACE == this->egl_surface) {
      throw egl_error_t(__PRETTY_FUNCTION__);
    }
  }
  ~egl_surface_t()
  {
    eglDestroySurface(this->egl_display, this->egl_surface);
  }

public:
  operator EGLSurface() const { return this->egl_surface; }

private:
  EGLSurface egl_surface;
  EGLDisplay egl_display;
};

//////////////////////////////////////////////////////////////////////////////

template <typename Ch>
inline std::basic_ostream<Ch>& egl_display_t::dump(std::basic_ostream<Ch>& output,
						   EGLConfig config)
{
#define MAKE_ENUM_AND_LABEL(x) std::tuple<EGLint, char const*>(x, #x)
  constexpr std::tuple<EGLint, char const*> enum_and_label[] = {
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
  };
#undef MAKE_ENUM_AND_LABEL

  for (auto const& item : enum_and_label) {
    output << std::get<1>(item) << '\t';

    EGLint value;
    if (eglGetConfigAttrib(this->egl_display, config, std::get<0>(item), &value)) {
      output << value << std::endl;
    }
    else {
      output << '-' << std::endl;
    }
  }
  return output;
}

#endif/*INCLUDE_WAYLAND_EGL_HELPER_HPP_0D31093B_03DD_44D7_AD20_81DE312BE7A7*/
