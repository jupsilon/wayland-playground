#ifndef INCLUDE_UTILITIES_MISC_HPP_961B24B7_35DC_4A01_9DE3_E5DEE9D0895B
#define INCLUDE_UTILITIES_MISC_HPP_961B24B7_35DC_4A01_9DE3_E5DEE9D0895B

#include <array>

namespace utilities
{
  template <typename T, typename ...Args>
  inline auto array(Args... args) {
    return std::array<T, sizeof... (args)>{ {args...} };
  }
}

#endif/*INCLUDE_UTILITIES_MISC_HPP_961B24B7_35DC_4A01_9DE3_E5DEE9D0895B*/
