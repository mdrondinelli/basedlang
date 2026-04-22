#include "bytecode/module_builder.h"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <utility>

namespace benson::bytecode
{

  namespace
  {

    constexpr auto unbound_label_offset =
      std::numeric_limits<std::ptrdiff_t>::max();

    auto fits_immediate(std::ptrdiff_t offset) -> bool
    {
      return offset >=
               std::numeric_limits<Immediate::Underlying_type>::min() &&
             offset <=
               std::numeric_limits<Immediate::Underlying_type>::max();
    }

    auto fits_wide_immediate(std::ptrdiff_t offset) -> bool
    {
      return offset >=
               std::numeric_limits<Wide_immediate::Underlying_type>::min() &&
             offset <=
               std::numeric_limits<Wide_immediate::Underlying_type>::max();
    }

  } // namespace

  Module_builder_storage::Code_output_stream::Code_output_stream(
    std::vector<std::byte> *code
  )
      : _code{code}
  {
  }

  void Module_builder_storage::Code_output_stream::write_bytes(
    std::span<std::byte const> buffer
  )
  {
    _code->insert(_code->end(), buffer.begin(), buffer.end());
  }

  Module_builder_storage::Module_builder_storage()
      : _module{}, _output_stream{&_module.code}
  {
  }

  Module_builder::Module_builder()
      : Module_builder_storage{}, Bytecode_writer{&_output_stream}
  {
  }

  auto Module_builder::make_label() -> Label
  {
    auto const label = Label{_label_offsets.size()};
    _label_offsets.push_back(unbound_label_offset);
    return label;
  }

  void Module_builder::place_label(Label label)
  {
    assert(label.id < _label_offsets.size());
    assert(_label_offsets[label.id] == unbound_label_offset);
    _label_offsets[label.id] = flushed_code_size();
  }

  void Module_builder::emit_jmp_i(Label target)
  {
    assert(target.id < _label_offsets.size());
    auto const instruction_offset = flushed_code_size();
    auto const target_offset = _label_offsets[target.id];
    if (target_offset != unbound_label_offset)
    {
      auto const narrow_offset = target_offset - (instruction_offset + 2);
      if (fits_immediate(narrow_offset))
      {
        Bytecode_writer::emit_jmp_i(
          Wide_immediate{static_cast<Wide_immediate::Underlying_type>(narrow_offset)}
        );
      }
      else
      {
        auto const wide_offset = target_offset - (instruction_offset + 4);
        if (!fits_wide_immediate(wide_offset))
        {
          throw std::runtime_error{"jmp_i target out of range"};
        }
        Bytecode_writer::emit_jmp_i(
          Wide_immediate{static_cast<Wide_immediate::Underlying_type>(wide_offset)}
        );
      }
    }
    else
    {
      constexpr auto wide_jump_dummy_offset = 0x0100;
      Bytecode_writer::emit_jmp_i(
        Wide_immediate{static_cast<Wide_immediate::Underlying_type>(wide_jump_dummy_offset)}
      );
      _pending_jumps.push_back(
        Pending_jump{
          .target = target,
          .immediate_offset = instruction_offset + 2,
          .next_instruction_offset = instruction_offset + 4,
        }
      );
    }
  }

  auto Module_builder::build() -> Module
  {
    Bytecode_writer::flush();
    for (auto const &jump : _pending_jumps)
    {
      assert(jump.target.id < _label_offsets.size());
      auto const target_offset = _label_offsets[jump.target.id];
      if (target_offset == unbound_label_offset)
      {
        throw std::runtime_error{"jmp_i target label was never placed"};
      }
      auto const offset = target_offset - jump.next_instruction_offset;
      if (!fits_wide_immediate(offset))
      {
        throw std::runtime_error{"jmp_i target out of range"};
      }
      auto const immediate = Wide_immediate{
        static_cast<Wide_immediate::Underlying_type>(offset)
      };
      _module.code[jump.immediate_offset] =
        static_cast<std::byte>(immediate.value);
      _module.code[jump.immediate_offset + 1] =
        static_cast<std::byte>(immediate.value >> 8);
    }
    auto built = Module{};
    std::swap(built, _module);
    return built;
  }

  auto Module_builder::flushed_code_size() -> std::ptrdiff_t
  {
    Bytecode_writer::flush();
    return static_cast<std::ptrdiff_t>(_module.code.size());
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
        return Wide_constant{static_cast<Wide_constant::Underlying_type>(i)};
      }
    }

    if (_module.constant_table.size() ==
        static_cast<std::size_t>(
          std::numeric_limits<Wide_constant::Underlying_type>::max()
        ) + 1)
    {
      throw std::runtime_error{"constant table out of range"};
    }

    auto const index = Wide_constant{
      static_cast<Wide_constant::Underlying_type>(_module.constant_table.size())
    };
    _module.constant_table.push_back(
      static_cast<std::ptrdiff_t>(_module.constant_data.size())
    );
    _module.constant_data
      .insert(_module.constant_data.end(), bytes.begin(), bytes.end());
    return index;
  }

} // namespace benson::bytecode
