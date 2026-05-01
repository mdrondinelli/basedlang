#include <ir/source_map.h>

namespace benson::ir
{

  namespace
  {

    void hash_combine(std::size_t *seed, std::size_t value)
    {
      *seed ^= value + 0x9e3779b9 + (*seed << 6) + (*seed >> 2);
    }

  } // namespace

  std::size_t
  Source_map::Instruction_site_hash::operator()(Instruction_site site) const
  {
    auto seed = std::size_t{};
    hash_combine(&seed, std::hash<Function const *>{}(site.function));
    hash_combine(&seed, std::hash<Basic_block const *>{}(site.block));
    hash_combine(&seed, std::hash<std::ptrdiff_t>{}(site.instruction_index));
    return seed;
  }

  std::size_t
  Source_map::Terminator_site_hash::operator()(Terminator_site site) const
  {
    auto seed = std::size_t{};
    hash_combine(&seed, std::hash<Function const *>{}(site.function));
    hash_combine(&seed, std::hash<Basic_block const *>{}(site.block));
    return seed;
  }

  void Source_map::add(Instruction_site site, Source_span span)
  {
    _instruction_spans.emplace(site, span);
  }

  void Source_map::add(Terminator_site site, Source_span span)
  {
    _terminator_spans.emplace(site, span);
  }

  std::optional<Source_span> Source_map::lookup(Instruction_site site) const
  {
    auto const it = _instruction_spans.find(site);
    if (it == _instruction_spans.end())
    {
      return std::nullopt;
    }
    return it->second;
  }

  std::optional<Source_span> Source_map::lookup(Terminator_site site) const
  {
    auto const it = _terminator_spans.find(site);
    if (it == _terminator_spans.end())
    {
      return std::nullopt;
    }
    return it->second;
  }

} // namespace benson::ir
