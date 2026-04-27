#include <ir/source_map.h>

namespace benson::ir
{

  namespace
  {

    bool same_site(Instruction_site lhs, Instruction_site rhs)
    {
      return lhs.function == rhs.function && lhs.block == rhs.block &&
             lhs.instruction_index == rhs.instruction_index;
    }

    bool same_site(Terminator_site lhs, Terminator_site rhs)
    {
      return lhs.function == rhs.function && lhs.block == rhs.block;
    }

  } // namespace

  void Source_map::add(Instruction_site site, Source_span span)
  {
    _entries.emplace_back(site, span);
  }

  void Source_map::add(Terminator_site site, Source_span span)
  {
    _entries.emplace_back(site, span);
  }

  std::optional<Source_span> Source_map::lookup(Instruction_site site) const
  {
    for (auto const &entry : _entries)
    {
      if (auto const *stored = std::get_if<Instruction_site>(&entry.first);
          stored != nullptr && same_site(*stored, site))
      {
        return entry.second;
      }
    }
    return std::nullopt;
  }

  std::optional<Source_span> Source_map::lookup(Terminator_site site) const
  {
    for (auto const &entry : _entries)
    {
      if (auto const *stored = std::get_if<Terminator_site>(&entry.first);
          stored != nullptr && same_site(*stored, site))
      {
        return entry.second;
      }
    }
    return std::nullopt;
  }

} // namespace benson::ir
