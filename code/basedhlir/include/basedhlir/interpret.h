#ifndef BASEDHLIR_INTERPRET_H
#define BASEDHLIR_INTERPRET_H

#include <cstdint>
#include <span>

#include "constant_value.h"
#include "hlir.h"

namespace basedhlir
{

  Constant_value interpret(
    Function const &function,
    std::span<Constant_value const> arguments,
    std::int32_t fuel = 100000
  );

} // namespace basedhlir

#endif
