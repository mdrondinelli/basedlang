#ifndef BENSON_IR_SOURCE_MAP_H
#define BENSON_IR_SOURCE_MAP_H

#include <cstddef>
#include <functional>
#include <optional>
#include <unordered_map>

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

    bool operator==(Instruction_site const &) const = default;
  };

  struct Terminator_site
  {
    Function const *function{};
    Basic_block const *block{};

    bool operator==(Terminator_site const &) const = default;
  };

  class Source_map
  {
  public:
    void add(Instruction_site site, Source_span span);

    void add(Terminator_site site, Source_span span);

    std::optional<Source_span> lookup(Instruction_site site) const;

    std::optional<Source_span> lookup(Terminator_site site) const;

  private:
    struct Instruction_site_hash
    {
      std::size_t operator()(Instruction_site site) const;
    };

    struct Terminator_site_hash
    {
      std::size_t operator()(Terminator_site site) const;
    };

    std::unordered_map<Instruction_site, Source_span, Instruction_site_hash>
      _instruction_spans;
    std::unordered_map<Terminator_site, Source_span, Terminator_site_hash>
      _terminator_spans;
  };

} // namespace benson::ir

#endif // BENSON_IR_SOURCE_MAP_H
