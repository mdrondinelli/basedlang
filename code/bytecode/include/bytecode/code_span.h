#ifndef BENSON_BYTECODE_CODE_SPAN_H
#define BENSON_BYTECODE_CODE_SPAN_H

#include <cstddef>

namespace benson::bytecode
{

  struct Code_span
  {
    std::ptrdiff_t position;
    std::ptrdiff_t length;
  };

}

#endif // BENSON_BYTECODE_CODE_SPAN_H
