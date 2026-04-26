#ifndef BENSON_BYTECODE_MODULE_H
#define BENSON_BYTECODE_MODULE_H

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "spelling/spelling.h"

namespace benson::bytecode
{

  enum class Scalar_type
  {
    int8,
    int16,
    int32,
    int64,
    float_,
    double_,
    bool_,
    void_,
  };

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
    std::unordered_map<Spelling, Function> functions;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_MODULE_H
