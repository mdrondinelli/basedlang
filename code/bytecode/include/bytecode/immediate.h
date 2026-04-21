#ifndef BENSON_BYTECODE_IMMEDIATE_H
#define BENSON_BYTECODE_IMMEDIATE_H

#include <cstdint>

namespace benson::bytecode
{
  using Immediate = std::int8_t;
  using Wide_immediate = std::int16_t;
}

#endif // BENSON_BYTECODE_IMMEDIATE_H
