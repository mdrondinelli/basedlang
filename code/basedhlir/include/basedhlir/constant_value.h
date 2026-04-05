#ifndef BASEDHLIR_CONSTANT_VALUE_H
#define BASEDHLIR_CONSTANT_VALUE_H

#include <cstdint>
#include <variant>

namespace basedhlir
{

  struct Type;

  struct Function;

  struct Void_value
  {
  };

  struct Type_value
  {
    Type *type;
  };

  struct Function_value
  {
    Function *function;
  };

  using Constant_value = std::variant<
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t,
    bool,
    Void_value,
    Type_value,
    Function_value
  >;

} // namespace basedhlir

#endif
