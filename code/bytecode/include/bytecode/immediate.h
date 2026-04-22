#ifndef BENSON_BYTECODE_IMMEDIATE_H
#define BENSON_BYTECODE_IMMEDIATE_H

#include <cstdint>

namespace benson::bytecode
{

  struct Immediate
  {
    using Underlying_type = std::int8_t;
    Underlying_type value;
  };

  struct Wide_immediate
  {
    using Underlying_type = std::int16_t;
    Underlying_type value;
  };

}

#endif // BENSON_BYTECODE_IMMEDIATE_H
