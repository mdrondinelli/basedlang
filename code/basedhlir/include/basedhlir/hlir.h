#ifndef BASEDHLIR_HLIR_H
#define BASEDHLIR_HLIR_H

#include <memory>
#include <string>
#include <vector>

#include "type.h"

namespace basedhlir
{

  struct Parameter;

  struct Function;

  struct Translation_unit
  {
    std::vector<std::unique_ptr<Function>> functions;
  };

  struct Function
  {
    Type *type;
    std::vector<Parameter> parameters;
  };

  struct Parameter
  {
    std::string name;
    bool is_mutable;
  };

} // namespace basedhlir

#endif
