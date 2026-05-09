#ifndef BASEDHLIR_INTERPRET_H
#define BASEDHLIR_INTERPRET_H

#include <cstdint>
#include <exception>
#include <span>

#include "constant_value.h"
#include "hlir.h"

namespace benson::hlir
{

  class Fuel_exhausted_error: public std::exception
  {
  public:
    char const *what() const noexcept override
    {
      return "compile-time evaluation ran out of fuel";
    }
  };

  Constant_value interpret(
    Function const &function,
    std::span<Constant_value const> arguments,
    std::int32_t &fuel
  );

  Constant_value interpret(
    Function const &function,
    std::span<Constant_value const> arguments
  );

} // namespace benson::hlir

#endif
