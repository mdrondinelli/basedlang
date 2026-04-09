#ifndef BASEDHLIR_CONSTANT_VALUE_H
#define BASEDHLIR_CONSTANT_VALUE_H

#include <cstdint>
#include <variant>

namespace bensonhlir
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
    float,
    double,
    bool,
    Void_value,
    Type_value,
    Function_value
  >;

} // namespace bensonhlir

#endif
