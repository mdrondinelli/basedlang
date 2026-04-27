#ifndef BENSON_IR_SOURCE_MAP_H
#define BENSON_IR_SOURCE_MAP_H

#include <cstddef>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include <source/source_span.h>

namespace benson::ir
{

  struct Function;
  struct Basic_block;

  struct Instruction_site
  {
    Function const *function{};
    Basic_block const *block{};
    std::ptrdiff_t instruction_index{};
  };

  struct Terminator_site
  {
    Function const *function{};
    Basic_block const *block{};
  };

  using Source_site = std::variant<Instruction_site, Terminator_site>;

  class Source_map
  {
  public:
    void add(Instruction_site site, Source_span span);

    void add(Terminator_site site, Source_span span);

    std::optional<Source_span> lookup(Instruction_site site) const;

    std::optional<Source_span> lookup(Terminator_site site) const;

  private:
    std::vector<std::pair<Source_site, Source_span>> _entries;
  };

} // namespace benson::ir

#endif // BENSON_IR_SOURCE_MAP_H
