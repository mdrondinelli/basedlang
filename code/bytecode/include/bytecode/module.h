#ifndef BENSON_BYTECODE_MODULE_H
#define BENSON_BYTECODE_MODULE_H

#include <cstddef>
#include <vector>

namespace benson::bytecode
{

  struct Module
  {
    std::vector<std::byte> code;
    std::vector<std::byte> constant_data;
    std::vector<std::ptrdiff_t> constant_table;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_MODULE_H
