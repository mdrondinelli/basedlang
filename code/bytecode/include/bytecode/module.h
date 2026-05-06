#ifndef BENSON_BYTECODE_MODULE_H
#define BENSON_BYTECODE_MODULE_H

#include <cstddef>
#include <unordered_map>
#include <vector>

#include <spelling/spelling.h>

#include "scalar_type.h"
#include "source_map.h"

namespace benson::bytecode
{

  struct Function
  {
    std::ptrdiff_t position;
    std::vector<Scalar_type> parameter_types;
    Scalar_type return_type;
  };

  struct Module
  {
    std::vector<std::byte> code;
    std::vector<std::byte> constant_data;
    std::vector<std::ptrdiff_t> constant_table;
    std::vector<Function> functions;
    std::unordered_map<Spelling, std::size_t> function_indices;
    Source_map source_map; // TODO: do something with this
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_MODULE_H
