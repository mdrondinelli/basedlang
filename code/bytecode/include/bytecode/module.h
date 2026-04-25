#ifndef BENSON_BYTECODE_MODULE_H
#define BENSON_BYTECODE_MODULE_H

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "ir/type.h"
#include "spelling/spelling.h"

namespace benson::bytecode
{

  struct Function
  {
    std::ptrdiff_t position;
    std::vector<ir::Type *> parameter_types;
    ir::Type *return_type;
  };

  struct Module
  {
    std::vector<std::byte> code;
    std::vector<std::byte> constant_data;
    std::vector<std::ptrdiff_t> constant_table;
    std::unordered_map<Spelling, Function> functions;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_MODULE_H
