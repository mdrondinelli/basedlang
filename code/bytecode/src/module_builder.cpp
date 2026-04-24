#include "bytecode/module_builder.h"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <utility>

namespace benson::bytecode
{

  std::optional<std::ptrdiff_t>
  Module_builder::Label_jump_target_provider::target(
    std::ptrdiff_t patchable_immediate_position
  ) const
  {
    auto const retval = _builder->_label_positions[_label.index];
    if (!retval)
    {
      _builder->_unresolved_jump_offsets.push_back({
        .position = patchable_immediate_position,
        .label = _label,
      });
    }
    return retval;
  }

  Module_builder::Vector_output_stream::Vector_output_stream(
    std::vector<std::byte> *code
  )
      : _code{code}
  {
  }

  void Module_builder::Vector_output_stream::write_bytes(
    std::span<std::byte const> buffer
  )
  {
    _code->insert(_code->end(), buffer.begin(), buffer.end());
  }

  Module_builder::Module_builder()
      : _module{}, _output_stream{&_module.code}, _writer{&_output_stream}
  {
  }

  Bytecode_writer &Module_builder::writer() noexcept
  {
    return _writer;
  }

  Module_builder::Label Module_builder::make_label()
  {
    auto const label = Label{_label_positions.size()};
    _label_positions.push_back(std::nullopt);
    return label;
  }

  void Module_builder::place_label(Label label)
  {
    assert(label.index < _label_positions.size());
    assert(!_label_positions[label.index]);
    _label_positions[label.index] = _writer.position();
  }

  Module_builder::Label_jump_target_provider
  Module_builder::label_target(Label label)
  {
    assert(label.index < _label_positions.size());
    return Label_jump_target_provider{this, label};
  }

  auto Module_builder::build() -> Module
  {
    _writer.flush();
    for (auto const &[position, label] : _unresolved_jump_offsets)
    {
      assert(label.index < _label_positions.size());
      auto const target = _label_positions[label.index];
      if (!target)
      {
        throw std::runtime_error{"jmp target never placed"};
      }
      auto const offset =
        *target - (position + sizeof(Wide_immediate::Underlying_type));
      if (!std::in_range<Wide_immediate::Underlying_type>(offset))
      {
        throw std::runtime_error{"jmp target out of range"};
      }
      _module.code[position] = static_cast<std::byte>(offset);
      _module.code[position + 1] = static_cast<std::byte>(offset >> 8);
    }
    auto built = Module{};
    std::swap(built, _module);
    return built;
  }

  auto Module_builder::constant(std::span<std::byte const> bytes)
    -> Wide_constant
  {
    for (auto i = std::size_t{}; i < _module.constant_table.size(); ++i)
    {
      auto const offset = _module.constant_table[i];
      auto const existing = std::span{
        _module.constant_data.data() + offset,
        bytes.size(),
      };
      if (offset + static_cast<std::ptrdiff_t>(bytes.size()) <=
            static_cast<std::ptrdiff_t>(_module.constant_data.size()) &&
          std::equal(bytes.begin(), bytes.end(), existing.begin()))
      {
        return Wide_constant{i};
      }
    }

    if (_module.constant_table.size() ==
        static_cast<std::size_t>(
          std::numeric_limits<Wide_constant::Underlying_type>::max()
        ) +
          1)
    {
      throw std::runtime_error{"constant table out of range"};
    }

    auto const index = Wide_constant{_module.constant_table.size()};
    _module.constant_table.push_back(
      static_cast<std::ptrdiff_t>(_module.constant_data.size())
    );
    _module.constant_data
      .insert(_module.constant_data.end(), bytes.begin(), bytes.end());
    return index;
  }

} // namespace benson::bytecode
