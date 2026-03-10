#ifndef BASEDINTERP_VALUE_H
#define BASEDINTERP_VALUE_H

#include <cstddef>
#include <cstdint>
#include <variant>

namespace basedinterp
{

  struct Function_value
  {
    std::size_t index;
  };

  struct Value
  {
    std::variant<std::int32_t, Function_value> data;
  };

} // namespace basedinterp

#endif // BASEDINTERP_VALUE_H
