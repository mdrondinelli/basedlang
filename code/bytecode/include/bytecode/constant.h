#ifndef BENSON_BYTECODE_CONSTANT_INDEX_H
#define BENSON_BYTECODE_CONSTANT_INDEX_H

#include <cstdint>

namespace benson::bytecode
{

  struct Constant
  {
    using Underlying_type = std::uint8_t;
    Underlying_type value;
  };

  struct Wide_constant
  {
    using Underlying_type = std::uint16_t;
    Underlying_type value;
  };

}

#endif // BENSON_BYTECODE_CONSTANT_INDEX_H
