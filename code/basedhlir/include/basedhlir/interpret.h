#ifndef BASEDHLIR_INTERPRET_H
#define BASEDHLIR_INTERPRET_H

#include <vector>

#include "constant_value.h"
#include "hlir.h"

namespace basedhlir
{

  Constant_value interpret(
    Function const &function,
    std::vector<Constant_value> const &arguments,
    int fuel = 100000
  );

} // namespace basedhlir

#endif
