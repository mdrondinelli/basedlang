#ifndef BASEDHLIR_INTERPRET_H
#define BASEDHLIR_INTERPRET_H

#include <cstdint>
#include <span>
#include <stdexcept>

#include "constant_value.h"
#include "hlir.h"

namespace basedhlir
{

  // TODO: use std::exception as the base class
  class Fuel_exhausted_error: public std::runtime_error
  {
  public:
    Fuel_exhausted_error()
        : std::runtime_error{"compile-time evaluation ran out of fuel"}
    {
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

} // namespace basedhlir

#endif
