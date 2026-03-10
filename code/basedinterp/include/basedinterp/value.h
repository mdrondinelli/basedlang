#ifndef BASEDINTERP_VALUE_H
#define BASEDINTERP_VALUE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <variant>

namespace basedinterp
{

  struct Function_value
  {
    std::size_t index;
  };

  struct Value;

  struct Pointer_value
  {
    std::weak_ptr<Value> target;
  };

  struct Reference_value
  {
    std::shared_ptr<Value> target;
  };

  struct Value
  {
    std::variant<std::int32_t, Function_value, Pointer_value, Reference_value>
      data;
  };

} // namespace basedinterp

#endif // BASEDINTERP_VALUE_H
