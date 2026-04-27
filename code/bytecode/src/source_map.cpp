#include <bytecode/source_map.h>

namespace benson::bytecode
{

  std::optional<Source_span> Source_map::lookup(std::ptrdiff_t position) const
  {
    for (const auto &entry : _entries)
    {
      auto const offset = position - entry.first.position;
      if (offset >= 0 && offset < entry.first.length)
      {
        return entry.second;
      }
    }
    return std::nullopt;
  }

}
