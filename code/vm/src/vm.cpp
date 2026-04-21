#include <memory>
#include <stdexcept>

#include "vm/vm.h"

namespace benson
{

  namespace
  {

    template <typename ConstantType>
    void run_lookup_k(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto k = ConstantType{};
      std::memcpy(&k, instruction_pointer, sizeof(k));
      instruction_pointer += sizeof(k);
      vm.set_register_value(dst, vm.lookup_constant(k));
    }

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

    template <typename T, typename Fn>
    void run_register_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Fn fn
    )
    {
      auto const dst_reg = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const lhs_reg = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const rhs_reg = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      vm.set_register_value<T>(
        dst_reg,
        fn(vm.get_register_value<T>(lhs_reg), vm.get_register_value<T>(rhs_reg))
      );
    }

    template <typename T, typename Fn>
    void run_constant_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Fn fn
    )
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const lhs = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const rhs = static_cast<bytecode::Constant>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      vm.set_register_value<T>(
        dst,
        fn(vm.get_register_value<T>(lhs), vm.get_constant_value<T>(rhs))
      );
    }

    template <typename T, typename Fn>
    void run_wide_constant_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Fn fn
    )
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const lhs = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto rhs = bytecode::Wide_constant{};
      std::memcpy(&rhs, instruction_pointer, sizeof(rhs));
      instruction_pointer += sizeof(rhs);
      vm.set_register_value<T>(
        dst,
        fn(vm.get_register_value<T>(lhs), vm.get_constant_value<T>(rhs))
      );
    }

    template <std::size_t N, typename OffsetType>
    void run_load(std::byte const *&ip, Virtual_machine &vm)
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*ip++)
      );
      auto const base = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*ip++)
      );
      auto offset = OffsetType{};
      std::memcpy(&offset, ip, sizeof(offset));
      ip += sizeof(offset);

      auto const decoded =
        Pointer{vm.get_register_value<std::uint64_t>(base)}.decode();
      std::byte const *src = nullptr;
      switch (decoded.space)
      {
      case Address_space::stack:
        {
          auto const addr = decoded.offset + offset;
          if (addr + N > vm.stack->size())
          {
            throw std::runtime_error{"load out of bounds"};
          }
          src = vm.stack->data() + addr;
        }
        break;
      case Address_space::constant:
        src = vm.constant_memory + decoded.offset + offset;
        break;
      default:
        throw std::runtime_error{"unsupported address space for load"};
      }
      auto value = std::uint64_t{};
      std::memcpy(&value, src, N);
      vm.registers[static_cast<std::size_t>(dst)] = value;
    }

    template <std::size_t N, typename OffsetType>
    void run_store(std::byte const *&ip, Virtual_machine &vm)
    {
      auto const src = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*ip++)
      );
      auto const base = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*ip++)
      );
      auto offset = OffsetType{};
      std::memcpy(&offset, ip, sizeof(offset));
      ip += sizeof(offset);

      auto const decoded =
        Pointer{vm.get_register_value<std::uint64_t>(base)}.decode();
      if (decoded.space == Address_space::constant)
      {
        throw std::runtime_error{"store to constant memory"};
      }
      if (decoded.space != Address_space::stack)
      {
        throw std::runtime_error{"unsupported address space for store"};
      }
      auto const addr = decoded.offset + offset;
      if (addr + N > vm.stack->size())
      {
        throw std::runtime_error{"store out of bounds"};
      }
      auto *dst = vm.stack->data() + addr;
      auto const value = vm.registers[static_cast<std::size_t>(src)];
      std::memcpy(dst, &value, N);
    }

  } // namespace

  Virtual_machine::Virtual_machine()
      : instruction_pointer{nullptr},
        constant_memory{nullptr},
        constant_table{nullptr},
        registers{},
        stack{std::make_unique<std::array<std::byte, 4096>>()}
  {
  }

  void Virtual_machine::load(bytecode::Module const &module)
  {
    instruction_pointer = module.code.data();
    constant_memory = module.constant_data.data();
    constant_table = module.constant_table.data();
  }

  void Virtual_machine::run()
  {
    for (;;)
    {
      auto const opcode = static_cast<bytecode::Opcode>(*instruction_pointer++);
      if (opcode == bytecode::Opcode::exit)
      {
        return;
      }
      dispatch(opcode);
    }
  }

  void Virtual_machine::dispatch(bytecode::Opcode opcode)
  {
    switch (opcode)
    {
    case bytecode::Opcode::wide:
      wide_dispatch(static_cast<bytecode::Opcode>(*instruction_pointer++));
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k<bytecode::Constant>(instruction_pointer, *this);
      break;
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
    case bytecode::Opcode::add_i8:
      run_register_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs + rhs);
        }
      );
      break;
    case bytecode::Opcode::add_i8_k:
      run_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs + rhs);
        }
      );
      break;
    case bytecode::Opcode::add_i16:
      run_register_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs + rhs);
        }
      );
      break;
    case bytecode::Opcode::add_i16_k:
      run_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs + rhs);
        }
      );
      break;
    case bytecode::Opcode::add_i32:
      run_register_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_i32_k:
      run_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_i64:
      run_register_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_i64_k:
      run_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_f32:
      run_register_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_f32_k:
      run_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_f64:
      run_register_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_f64_k:
      run_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_i8:
      run_register_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs - rhs);
        }
      );
      break;
    case bytecode::Opcode::sub_i8_k:
      run_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs - rhs);
        }
      );
      break;
    case bytecode::Opcode::sub_i16:
      run_register_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs - rhs);
        }
      );
      break;
    case bytecode::Opcode::sub_i16_k:
      run_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs - rhs);
        }
      );
      break;
    case bytecode::Opcode::sub_i32:
      run_register_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_i32_k:
      run_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_i64:
      run_register_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_i64_k:
      run_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_f32:
      run_register_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_f32_k:
      run_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_f64:
      run_register_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_f64_k:
      run_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_i8:
      run_register_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs * rhs);
        }
      );
      break;
    case bytecode::Opcode::mul_i8_k:
      run_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs * rhs);
        }
      );
      break;
    case bytecode::Opcode::mul_i16:
      run_register_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs * rhs);
        }
      );
      break;
    case bytecode::Opcode::mul_i16_k:
      run_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs * rhs);
        }
      );
      break;
    case bytecode::Opcode::mul_i32:
      run_register_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_i32_k:
      run_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_i64:
      run_register_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_i64_k:
      run_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_f32:
      run_register_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_f32_k:
      run_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_f64:
      run_register_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_f64_k:
      run_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::div_i8:
      run_register_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs / rhs);
        }
      );
      break;
    case bytecode::Opcode::div_i8_k:
      run_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs / rhs);
        }
      );
      break;
    case bytecode::Opcode::div_i16:
      run_register_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs / rhs);
        }
      );
      break;
    case bytecode::Opcode::div_i16_k:
      run_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs / rhs);
        }
      );
      break;
    case bytecode::Opcode::div_i32:
      run_register_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_i32_k:
      run_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_i64:
      run_register_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_i64_k:
      run_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_f32:
      run_register_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_f32_k:
      run_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_f64:
      run_register_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_f64_k:
      run_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::mod_i8:
      run_register_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs % rhs);
        }
      );
      break;
    case bytecode::Opcode::mod_i8_k:
      run_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs % rhs);
        }
      );
      break;
    case bytecode::Opcode::mod_i16:
      run_register_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs % rhs);
        }
      );
      break;
    case bytecode::Opcode::mod_i16_k:
      run_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs % rhs);
        }
      );
      break;
    case bytecode::Opcode::mod_i32:
      run_register_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs % rhs;
        }
      );
      break;
    case bytecode::Opcode::mod_i32_k:
      run_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs % rhs;
        }
      );
      break;
    case bytecode::Opcode::mod_i64:
      run_register_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs % rhs;
        }
      );
      break;
    case bytecode::Opcode::mod_i64_k:
      run_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs % rhs;
        }
      );
      break;
    case bytecode::Opcode::load_8:
      run_load<1, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_16:
      run_load<2, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_32:
      run_load<4, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_64:
      run_load<8, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_8:
      run_store<1, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_16:
      run_store<2, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_32:
      run_store<4, bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_64:
      run_store<8, bytecode::Constant>(instruction_pointer, *this);
      break;
    default:
      throw std::runtime_error{"unimplemented bytecode opcode"};
    }
  }

  void Virtual_machine::wide_dispatch(bytecode::Opcode opcode)
  {
    switch (opcode)
    {
    case bytecode::Opcode::lookup_k:
      run_lookup_k<bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::add_i8_k:
      run_wide_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs + rhs);
        }
      );
      break;
    case bytecode::Opcode::add_i16_k:
      run_wide_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs + rhs);
        }
      );
      break;
    case bytecode::Opcode::add_i32_k:
      run_wide_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_i64_k:
      run_wide_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_f32_k:
      run_wide_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::add_f64_k:
      run_wide_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs + rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_i8_k:
      run_wide_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs - rhs);
        }
      );
      break;
    case bytecode::Opcode::sub_i16_k:
      run_wide_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs - rhs);
        }
      );
      break;
    case bytecode::Opcode::sub_i32_k:
      run_wide_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_i64_k:
      run_wide_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_f32_k:
      run_wide_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::sub_f64_k:
      run_wide_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs - rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_i8_k:
      run_wide_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs * rhs);
        }
      );
      break;
    case bytecode::Opcode::mul_i16_k:
      run_wide_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs * rhs);
        }
      );
      break;
    case bytecode::Opcode::mul_i32_k:
      run_wide_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_i64_k:
      run_wide_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_f32_k:
      run_wide_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::mul_f64_k:
      run_wide_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs * rhs;
        }
      );
      break;
    case bytecode::Opcode::div_i8_k:
      run_wide_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs / rhs);
        }
      );
      break;
    case bytecode::Opcode::div_i16_k:
      run_wide_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs / rhs);
        }
      );
      break;
    case bytecode::Opcode::div_i32_k:
      run_wide_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_i64_k:
      run_wide_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_f32_k:
      run_wide_constant_binary<float>(
        instruction_pointer,
        *this,
        [](float lhs, float rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::div_f64_k:
      run_wide_constant_binary<double>(
        instruction_pointer,
        *this,
        [](double lhs, double rhs)
        {
          return lhs / rhs;
        }
      );
      break;
    case bytecode::Opcode::mod_i8_k:
      run_wide_constant_binary<std::int8_t>(
        instruction_pointer,
        *this,
        [](std::int8_t lhs, std::int8_t rhs)
        {
          return static_cast<std::int8_t>(lhs % rhs);
        }
      );
      break;
    case bytecode::Opcode::mod_i16_k:
      run_wide_constant_binary<std::int16_t>(
        instruction_pointer,
        *this,
        [](std::int16_t lhs, std::int16_t rhs)
        {
          return static_cast<std::int16_t>(lhs % rhs);
        }
      );
      break;
    case bytecode::Opcode::mod_i32_k:
      run_wide_constant_binary<std::int32_t>(
        instruction_pointer,
        *this,
        [](std::int32_t lhs, std::int32_t rhs)
        {
          return lhs % rhs;
        }
      );
      break;
    case bytecode::Opcode::mod_i64_k:
      run_wide_constant_binary<std::int64_t>(
        instruction_pointer,
        *this,
        [](std::int64_t lhs, std::int64_t rhs)
        {
          return lhs % rhs;
        }
      );
      break;
    case bytecode::Opcode::load_8:
      run_load<1, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_16:
      run_load<2, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_32:
      run_load<4, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_64:
      run_load<8, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_8:
      run_store<1, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_16:
      run_store<2, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_32:
      run_store<4, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_64:
      run_store<8, bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    default:
      throw std::runtime_error{"unimplemented wide bytecode opcode"};
    }
  }
} // namespace benson
