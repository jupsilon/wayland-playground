
#ifndef INCLUDE_WAYLAND_CLIENT_HELPER_HPP_52436FEF_3A50_44D8_991D_F2E0525AB182
#define INCLUDE_WAYLAND_CLIENT_HELPER_HPP_52436FEF_3A50_44D8_991D_F2E0525AB182

#include <tuple>
#include <cstring>
#include <memory>

#include <wayland-client.h>
#include "weston-desktop-shell-client.h"

template <typename T, typename D>
inline auto attach_unique(T* ptr, D del) {
  return std::unique_ptr<T, D>(ptr, del);
}

template <typename T>
inline auto attach_unique(T* ptr) {
  return attach_unique(ptr, [](T* p) { wl_proxy_destroy(reinterpret_cast<wl_proxy*>(p)); });
}

template <typename T> constexpr wl_interface const* const iface = nullptr;
template <> constexpr wl_interface const* const iface<wl_compositor>        = &wl_compositor_interface;
template <> constexpr wl_interface const* const iface<wl_shell>             = &wl_shell_interface;
template <> constexpr wl_interface const* const iface<wl_seat>              = &wl_seat_interface;
template <> constexpr wl_interface const* const iface<wl_output>            = &wl_output_interface;
template <> constexpr wl_interface const* const iface<weston_desktop_shell> = &weston_desktop_shell_interface;

template <typename T>
inline auto global_bind(wl_display* display, uint32_t version) {
  static_assert(nullptr != iface<T>, "iface not defined");

  using userdata_type = std::tuple<T*, uint32_t>;
  auto registry = attach_unique(wl_display_get_registry(display));
  wl_registry_listener listener = {
    .global = [](void* data,
		 wl_registry* registry,
		 uint32_t name,
		 char const* interface,
		 uint32_t version)
    {
      if (0 == std::strcmp(interface, iface<T>->name)) {
	auto& userdata = *(reinterpret_cast<userdata_type*>(data));

	std::get<0>(userdata) =
	reinterpret_cast<T*>(wl_registry_bind(registry,
					      name,
					      iface<T>,
					      std::min(version,
						       std::get<1>(userdata))));
      }
    },
    .global_remove = [](void* data,
			wl_registry* registry,
			uint32_t name)
    {
    },
  };

  userdata_type userdata(nullptr, version);

  wl_registry_add_listener(registry.get(), &listener, &userdata);
  wl_display_roundtrip(display);

  return attach_unique(std::get<0>(userdata));
}

#endif/*INCLUDE_WAYLAND_CLIENT_HELPER_HPP_52436FEF_3A50_44D8_991D_F2E0525AB182*/
