#ifndef BENSON_BYTECODE_SOURCE_MAP_H
#define BENSON_BYTECODE_SOURCE_MAP_H

#include <optional>
#include <utility>
#include <vector>

#include <source/source_span.h>

#include "code_span.h"

namespace benson::bytecode
{

  class Source_map
  {
  public:
    std::optional<Source_span> lookup(std::ptrdiff_t position) const;

  private:
    std::vector<std::pair<Code_span, Source_span>> _entries;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_SOURCE_MAP_H
