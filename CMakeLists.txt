
cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS
  "-std=c++1z -Wall"
  )

set(SCANNER
  "wayland-scanner"
  )
set(SHELL_PROTOCOL
  "weston-desktop-shell"
  )

add_executable(weston-shell-client
  weston-shell-client.cc
  ${SHELL_PROTOCOL}.c
  )

add_executable(sample-client
  sample-client.cc
  ${SHELL_PROTOCOL}.c
  )

target_link_libraries(weston-shell-client
  wayland-client
  wayland-egl
  GL
  EGL
  )

target_link_libraries(sample-client
  wayland-client
  wayland-egl
  GL
  EGL
  )

include_directories(
  ${CMAKE_BINARY_DIR}
  )

add_custom_command(
  OUTPUT ${SHELL_PROTOCOL}.c
  COMMAND ${SCANNER} code          < ${CMAKE_SOURCE_DIR}/${SHELL_PROTOCOL}.xml > ${SHELL_PROTOCOL}.c
  COMMAND ${SCANNER} client-header < ${CMAKE_SOURCE_DIR}/${SHELL_PROTOCOL}.xml > ${SHELL_PROTOCOL}-client.h
  DEPENDS ${SHELL_PROTOCOL}.xml
  )

add_custom_target(run
  COMMAND weston
  DEPENDS weston-shell-client
  )
