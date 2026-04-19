#include <memory>
#include <stdexcept>

#include <bytecode/opcode.h>
#include <bytecode/register.h>

#include "vm/vm.h"

namespace benson
{

  namespace
  {
    template <typename T>
    void run_neg(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst_reg = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const src_reg = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      vm.set_register_value<T>(dst_reg, -vm.get_register_value<T>(src_reg));
    }

  } // namespace

  Virtual_machine::Virtual_machine()
      : instruction_pointer{nullptr},
        registers{},
        stack{std::make_unique<std::array<std::byte, 4096>>()}
  {
  }

  void Virtual_machine::run()
  {
    for (;;)
    {
      auto const opcode = static_cast<bytecode::Opcode>(*instruction_pointer++);
      switch (opcode)
      {
      case bytecode::Opcode::nop:
        break;
      case bytecode::Opcode::exit:
        return;
      case bytecode::Opcode::neg_i8:
        run_neg<std::int8_t>(instruction_pointer, *this);
        break;
      case bytecode::Opcode::neg_i16:
        run_neg<std::int16_t>(instruction_pointer, *this);
        break;
      case bytecode::Opcode::neg_i32:
        run_neg<std::int32_t>(instruction_pointer, *this);
        break;
      case bytecode::Opcode::neg_i64:
        run_neg<std::int64_t>(instruction_pointer, *this);
        break;
      case bytecode::Opcode::neg_f32:
        run_neg<float>(instruction_pointer, *this);
        break;
      case bytecode::Opcode::neg_f64:
        run_neg<double>(instruction_pointer, *this);
        break;
      default:
        throw std::runtime_error{"unimplemented bytecode opcode"};
      }
    }
  }

} // namespace benson
