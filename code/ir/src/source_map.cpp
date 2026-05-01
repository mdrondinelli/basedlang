#include <hashing/hash.h>
#include <ir/source_map.h>

namespace benson::ir
{

  std::size_t
  Source_map::Instruction_site_hash::operator()(Instruction_site site) const
  {
    return static_cast<std::size_t>(
      hash_values(site.function, site.block, site.instruction_index)
    );
  }

  std::size_t
  Source_map::Terminator_site_hash::operator()(Terminator_site site) const
  {
    return static_cast<std::size_t>(hash_values(site.function, site.block));
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
