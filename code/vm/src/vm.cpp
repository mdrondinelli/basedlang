#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "bytecode/immediate.h"
#include "vm/vm.h"

namespace benson
{

  namespace
  {

    enum class Operand_width
    {
      narrow,
      wide,
    };

    bytecode::Register
    read_register(std::byte const *&instruction_pointer, Operand_width width)
    {
      if (width == Operand_width::narrow)
      {
        return bytecode::Register{
          static_cast<std::uint8_t>(*instruction_pointer++)
        };
      }
      auto value = bytecode::Register::Underlying_type{};
      std::memcpy(&value, instruction_pointer, sizeof(value));
      instruction_pointer += sizeof(value);
      return bytecode::Register{value};
    }

    bytecode::Constant
    read_constant(std::byte const *&instruction_pointer, Operand_width width)
    {
      if (width == Operand_width::narrow)
      {
        return bytecode::Constant{
          static_cast<std::uint8_t>(*instruction_pointer++)
        };
      }
      auto value = bytecode::Constant::Underlying_type{};
      std::memcpy(&value, instruction_pointer, sizeof(value));
      instruction_pointer += sizeof(value);
      return bytecode::Constant{value};
    }

    bytecode::Immediate
    read_immediate(std::byte const *&instruction_pointer, Operand_width width)
    {
      if (width == Operand_width::narrow)
      {
        return bytecode::Immediate{
          static_cast<std::int8_t>(*instruction_pointer++)
        };
      }
      auto value = bytecode::Immediate::Underlying_type{};
      std::memcpy(&value, instruction_pointer, sizeof(value));
      instruction_pointer += sizeof(value);
      return bytecode::Immediate{value};
    }

    void run_lookup_k(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const k = read_constant(instruction_pointer, width);
      vm.set_register_value(dst, vm.lookup_constant(k));
    }

    void run_mov(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const src = read_register(instruction_pointer, width);
      (*vm.registers)[dst.value] = (*vm.registers)[src.value];
    }

    void run_mov_i(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const src = read_immediate(instruction_pointer, width);
      vm.set_register_value(dst, src.value);
    }

    template <typename CppType, typename Fn>
    void run_register_unary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const src = read_register(instruction_pointer, width);
      vm.set_register_value(dst, Fn{}(vm.get_register_value<CppType>(src)));
    }

    template <typename OperandType, typename Fn>
    void run_register_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const lhs = read_register(instruction_pointer, width);
      auto const rhs = read_register(instruction_pointer, width);
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          vm.get_register_value<OperandType>(rhs)
        )
      );
    }

    template <typename OperandType, typename Fn>
    void run_constant_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const lhs = read_register(instruction_pointer, width);
      auto const rhs = read_constant(instruction_pointer, width);
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          vm.get_constant_value<OperandType>(rhs)
        )
      );
    }

    template <typename OperandType, typename Fn>
    void run_immediate_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const lhs = read_register(instruction_pointer, width);
      auto const rhs = read_immediate(instruction_pointer, width);
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          static_cast<OperandType>(rhs.value)
        )
      );
    }

    struct Sx_fn
    {
      template <typename T>
      [[nodiscard]] std::int64_t operator()(T value) const
      {
        return static_cast<std::int64_t>(value);
      }
    };

    struct Neg_fn
    {
      template <typename T>
      [[nodiscard]] T operator()(T value) const
      {
        return -value;
      }
    };

    struct Add_fn
    {
      template <typename T>
      [[nodiscard]] T operator()(T lhs, T rhs) const
      {
        return lhs + rhs;
      }
    };

    struct Sub_fn
    {
      template <typename T>
      [[nodiscard]] T operator()(T lhs, T rhs) const
      {
        return lhs - rhs;
      }
    };

    struct Mul_fn
    {
      template <typename T>
      [[nodiscard]] T operator()(T lhs, T rhs) const
      {
        return lhs * rhs;
      }
    };

    struct Div_fn
    {
      template <typename T>
      [[nodiscard]] T operator()(T lhs, T rhs) const
      {
        return lhs / rhs;
      }
    };

    struct Mod_fn
    {
      template <typename T>
      [[nodiscard]] T operator()(T lhs, T rhs) const
      {
        return lhs % rhs;
      }
    };

    struct Cmp_eq_fn
    {
      template <typename T>
      [[nodiscard]] bool operator()(T lhs, T rhs) const
      {
        return lhs == rhs;
      }
    };

    struct Cmp_ne_fn
    {
      template <typename T>
      [[nodiscard]] bool operator()(T lhs, T rhs) const
      {
        return lhs != rhs;
      }
    };

    struct Cmp_lt_fn
    {
      template <typename T>
      [[nodiscard]] bool operator()(T lhs, T rhs) const
      {
        return lhs < rhs;
      }
    };

    struct Cmp_le_fn
    {
      template <typename T>
      [[nodiscard]] bool operator()(T lhs, T rhs) const
      {
        return lhs <= rhs;
      }
    };

    struct Cmp_gt_fn
    {
      template <typename T>
      [[nodiscard]] bool operator()(T lhs, T rhs) const
      {
        return lhs > rhs;
      }
    };

    struct Cmp_ge_fn
    {
      template <typename T>
      [[nodiscard]] bool operator()(T lhs, T rhs) const
      {
        return lhs >= rhs;
      }
    };

    template <std::size_t N>
    void run_load(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const dst = read_register(instruction_pointer, width);
      auto const base = read_register(instruction_pointer, width);
      auto const offset = read_immediate(instruction_pointer, width);

      auto [address_space, base_address] =
        Pointer{vm.get_register_value<std::uint64_t>(base)}.decode();
      auto const space_pointer = [&]() -> std::byte const *
      {
        switch (address_space)
        {
        case Address_space::constant:
          return vm.constant_memory;
        case Address_space::stack:
          return vm.stack->data();
        default:
          throw std::runtime_error{"unsupported address space for load"};
        }
      }();

      auto value = std::uint64_t{};
      // TODO: bounds check
      std::memcpy(&value, space_pointer + base_address + offset.value, N);
      (*vm.registers)[dst.value] = value;
    }

    template <std::size_t N>
    void run_store(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const src = read_register(instruction_pointer, width);
      auto const base = read_register(instruction_pointer, width);
      auto const offset = read_immediate(instruction_pointer, width);

      auto const [address_space, base_address] =
        Pointer{vm.get_register_value<std::uint64_t>(base)}.decode();
      if (address_space == Address_space::constant)
      {
        throw std::runtime_error{"store to constant memory"};
      }
      if (address_space != Address_space::stack)
      {
        throw std::runtime_error{"unsupported address space for store"};
      }

      // TODO: bounds check
      std::memcpy(
        vm.stack->data() + base_address + offset.value,
        &(*vm.registers)[src.value],
        N
      );
    }

    void run_jmp_i(std::byte const *&instruction_pointer, Operand_width width)
    {
      auto const offset = read_immediate(instruction_pointer, width);
      instruction_pointer += offset.value;
    }

    void push_u64(Virtual_machine &vm, std::uint64_t value)
    {
      auto const sp = vm.get_register_value<Pointer>(bytecode::sp).decode();
      if (sp.space != Address_space::stack)
      {
        throw std::runtime_error{"unsupported address space for call stack"};
      }
      auto const new_offset = sp.offset - sizeof(std::uint64_t);
      std::memcpy(vm.stack->data() + new_offset, &value, sizeof(value));
      vm.set_register_value(
        bytecode::sp,
        Pointer{Address_space::stack, new_offset}
      );
    }

    auto pop_u64(Virtual_machine &vm) -> std::uint64_t
    {
      auto const sp = vm.get_register_value<Pointer>(bytecode::sp).decode();
      if (sp.space != Address_space::stack)
      {
        throw std::runtime_error{"unsupported address space for call stack"};
      }
      auto value = std::uint64_t{};
      std::memcpy(&value, vm.stack->data() + sp.offset, sizeof(value));
      vm.set_register_value(
        bytecode::sp,
        Pointer{Address_space::stack, sp.offset + sizeof(std::uint64_t)}
      );
      return value;
    }

    void run_call_i(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const offset = read_immediate(instruction_pointer, width);
      push_u64(
        vm,
        static_cast<std::uint64_t>(
          reinterpret_cast<std::uintptr_t>(instruction_pointer)
        )
      );
      instruction_pointer += offset.value;
    }

    void run_ret(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      instruction_pointer = reinterpret_cast<std::byte const *>(pop_u64(vm));
    }

    void run_jnz_i(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm,
      Operand_width width
    )
    {
      auto const src = read_register(instruction_pointer, width);
      auto const offset = read_immediate(instruction_pointer, width);
      if (vm.get_register_value<std::int32_t>(src) != 0)
      {
        instruction_pointer += offset.value;
      }
    }

  } // namespace

  Virtual_machine::Virtual_machine()
      : instruction_pointer{nullptr},
        constant_memory{nullptr},
        constant_table{nullptr},
        registers{std::make_unique<std::array<std::uint64_t, 64 * 1024>>()},
        stack{std::make_unique<std::array<std::byte, 16 * 1024 * 1024>>()}
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
    set_register_value(
      bytecode::sp,
      Pointer{Address_space::stack, stack->size()}
    );
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
    case bytecode::Opcode::jmp_i:
      run_jmp_i(instruction_pointer, Operand_width::narrow);
      break;
    case bytecode::Opcode::call_i:
      run_call_i(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::ret:
      run_ret(instruction_pointer, *this);
      break;
    case bytecode::Opcode::jnz_i:
      run_jnz_i(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::mov:
      run_mov(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::mov_i:
      run_mov_i(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::load_8:
      run_load<1>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::load_16:
      run_load<2>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::load_32:
      run_load<4>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::load_64:
      run_load<8>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::store_8:
      run_store<1>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::store_16:
      run_store<2>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::store_32:
      run_store<4>(instruction_pointer, *this, Operand_width::narrow);
      break;
    case bytecode::Opcode::store_64:
      run_store<8>(instruction_pointer, *this, Operand_width::narrow);
      break;

#define UNARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:     \
    run_register_unary<type, fn>(    \
      instruction_pointer,           \
      *this,                         \
      Operand_width::narrow          \
    );                               \
    break;
#define REGISTER_BINARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:               \
    run_register_binary<type, fn>(             \
      instruction_pointer,                     \
      *this,                                   \
      Operand_width::narrow                    \
    );                                         \
    break;
#define CONSTANT_BINARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:               \
    run_constant_binary<type, fn>(             \
      instruction_pointer,                     \
      *this,                                   \
      Operand_width::narrow                    \
    );                                         \
    break;
#define IMMEDIATE_BINARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:                \
    run_immediate_binary<type, fn>(             \
      instruction_pointer,                      \
      *this,                                    \
      Operand_width::narrow                     \
    );                                          \
    break;

      // sx
      UNARY_CASE(sx_8, std::int8_t, Sx_fn)
      UNARY_CASE(sx_16, std::int16_t, Sx_fn)
      UNARY_CASE(sx_32, std::int32_t, Sx_fn)
      // neg
      UNARY_CASE(neg_i32, std::int32_t, Neg_fn)
      UNARY_CASE(neg_i64, std::int64_t, Neg_fn)
      UNARY_CASE(neg_f32, float, Neg_fn)
      UNARY_CASE(neg_f64, double, Neg_fn)
      // add
      REGISTER_BINARY_CASE(add_i32, std::int32_t, Add_fn)
      CONSTANT_BINARY_CASE(add_i32_k, std::int32_t, Add_fn)
      IMMEDIATE_BINARY_CASE(add_i32_i, std::int32_t, Add_fn)
      REGISTER_BINARY_CASE(add_i64, std::int64_t, Add_fn)
      CONSTANT_BINARY_CASE(add_i64_k, std::int64_t, Add_fn)
      IMMEDIATE_BINARY_CASE(add_i64_i, std::int64_t, Add_fn)
      REGISTER_BINARY_CASE(add_f32, float, Add_fn)
      CONSTANT_BINARY_CASE(add_f32_k, float, Add_fn)
      REGISTER_BINARY_CASE(add_f64, double, Add_fn)
      CONSTANT_BINARY_CASE(add_f64_k, double, Add_fn)
      // sub
      REGISTER_BINARY_CASE(sub_i32, std::int32_t, Sub_fn)
      CONSTANT_BINARY_CASE(sub_i32_k, std::int32_t, Sub_fn)
      IMMEDIATE_BINARY_CASE(sub_i32_i, std::int32_t, Sub_fn)
      REGISTER_BINARY_CASE(sub_i64, std::int64_t, Sub_fn)
      CONSTANT_BINARY_CASE(sub_i64_k, std::int64_t, Sub_fn)
      IMMEDIATE_BINARY_CASE(sub_i64_i, std::int64_t, Sub_fn)
      REGISTER_BINARY_CASE(sub_f32, float, Sub_fn)
      CONSTANT_BINARY_CASE(sub_f32_k, float, Sub_fn)
      REGISTER_BINARY_CASE(sub_f64, double, Sub_fn)
      CONSTANT_BINARY_CASE(sub_f64_k, double, Sub_fn)
      // mul
      REGISTER_BINARY_CASE(mul_i32, std::int32_t, Mul_fn)
      CONSTANT_BINARY_CASE(mul_i32_k, std::int32_t, Mul_fn)
      IMMEDIATE_BINARY_CASE(mul_i32_i, std::int32_t, Mul_fn)
      REGISTER_BINARY_CASE(mul_i64, std::int64_t, Mul_fn)
      CONSTANT_BINARY_CASE(mul_i64_k, std::int64_t, Mul_fn)
      IMMEDIATE_BINARY_CASE(mul_i64_i, std::int64_t, Mul_fn)
      REGISTER_BINARY_CASE(mul_f32, float, Mul_fn)
      CONSTANT_BINARY_CASE(mul_f32_k, float, Mul_fn)
      REGISTER_BINARY_CASE(mul_f64, double, Mul_fn)
      CONSTANT_BINARY_CASE(mul_f64_k, double, Mul_fn)
      // div
      REGISTER_BINARY_CASE(div_i32, std::int32_t, Div_fn)
      CONSTANT_BINARY_CASE(div_i32_k, std::int32_t, Div_fn)
      IMMEDIATE_BINARY_CASE(div_i32_i, std::int32_t, Div_fn)
      REGISTER_BINARY_CASE(div_i64, std::int64_t, Div_fn)
      CONSTANT_BINARY_CASE(div_i64_k, std::int64_t, Div_fn)
      IMMEDIATE_BINARY_CASE(div_i64_i, std::int64_t, Div_fn)
      REGISTER_BINARY_CASE(div_f32, float, Div_fn)
      CONSTANT_BINARY_CASE(div_f32_k, float, Div_fn)
      REGISTER_BINARY_CASE(div_f64, double, Div_fn)
      CONSTANT_BINARY_CASE(div_f64_k, double, Div_fn)
      // mod
      REGISTER_BINARY_CASE(mod_i32, std::int32_t, Mod_fn)
      CONSTANT_BINARY_CASE(mod_i32_k, std::int32_t, Mod_fn)
      IMMEDIATE_BINARY_CASE(mod_i32_i, std::int32_t, Mod_fn)
      REGISTER_BINARY_CASE(mod_i64, std::int64_t, Mod_fn)
      CONSTANT_BINARY_CASE(mod_i64_k, std::int64_t, Mod_fn)
      IMMEDIATE_BINARY_CASE(mod_i64_i, std::int64_t, Mod_fn)
      // cmp_eq
      REGISTER_BINARY_CASE(cmp_eq_i32, std::int32_t, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_i32_k, std::int32_t, Cmp_eq_fn)
      IMMEDIATE_BINARY_CASE(cmp_eq_i32_i, std::int32_t, Cmp_eq_fn)
      REGISTER_BINARY_CASE(cmp_eq_i64, std::int64_t, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_i64_k, std::int64_t, Cmp_eq_fn)
      IMMEDIATE_BINARY_CASE(cmp_eq_i64_i, std::int64_t, Cmp_eq_fn)
      REGISTER_BINARY_CASE(cmp_eq_f32, float, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_f32_k, float, Cmp_eq_fn)
      REGISTER_BINARY_CASE(cmp_eq_f64, double, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_f64_k, double, Cmp_eq_fn)
      // cmp_ne
      REGISTER_BINARY_CASE(cmp_ne_i32, std::int32_t, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_i32_k, std::int32_t, Cmp_ne_fn)
      IMMEDIATE_BINARY_CASE(cmp_ne_i32_i, std::int32_t, Cmp_ne_fn)
      REGISTER_BINARY_CASE(cmp_ne_i64, std::int64_t, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_i64_k, std::int64_t, Cmp_ne_fn)
      IMMEDIATE_BINARY_CASE(cmp_ne_i64_i, std::int64_t, Cmp_ne_fn)
      REGISTER_BINARY_CASE(cmp_ne_f32, float, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_f32_k, float, Cmp_ne_fn)
      REGISTER_BINARY_CASE(cmp_ne_f64, double, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_f64_k, double, Cmp_ne_fn)
      // cmp_lt
      REGISTER_BINARY_CASE(cmp_lt_i32, std::int32_t, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_i32_k, std::int32_t, Cmp_lt_fn)
      IMMEDIATE_BINARY_CASE(cmp_lt_i32_i, std::int32_t, Cmp_lt_fn)
      REGISTER_BINARY_CASE(cmp_lt_i64, std::int64_t, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_i64_k, std::int64_t, Cmp_lt_fn)
      IMMEDIATE_BINARY_CASE(cmp_lt_i64_i, std::int64_t, Cmp_lt_fn)
      REGISTER_BINARY_CASE(cmp_lt_f32, float, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_f32_k, float, Cmp_lt_fn)
      REGISTER_BINARY_CASE(cmp_lt_f64, double, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_f64_k, double, Cmp_lt_fn)
      // cmp_le
      REGISTER_BINARY_CASE(cmp_le_i32, std::int32_t, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_i32_k, std::int32_t, Cmp_le_fn)
      IMMEDIATE_BINARY_CASE(cmp_le_i32_i, std::int32_t, Cmp_le_fn)
      REGISTER_BINARY_CASE(cmp_le_i64, std::int64_t, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_i64_k, std::int64_t, Cmp_le_fn)
      IMMEDIATE_BINARY_CASE(cmp_le_i64_i, std::int64_t, Cmp_le_fn)
      REGISTER_BINARY_CASE(cmp_le_f32, float, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_f32_k, float, Cmp_le_fn)
      REGISTER_BINARY_CASE(cmp_le_f64, double, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_f64_k, double, Cmp_le_fn)
      // cmp_gt
      REGISTER_BINARY_CASE(cmp_gt_i32, std::int32_t, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_i32_k, std::int32_t, Cmp_gt_fn)
      IMMEDIATE_BINARY_CASE(cmp_gt_i32_i, std::int32_t, Cmp_gt_fn)
      REGISTER_BINARY_CASE(cmp_gt_i64, std::int64_t, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_i64_k, std::int64_t, Cmp_gt_fn)
      IMMEDIATE_BINARY_CASE(cmp_gt_i64_i, std::int64_t, Cmp_gt_fn)
      REGISTER_BINARY_CASE(cmp_gt_f32, float, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_f32_k, float, Cmp_gt_fn)
      REGISTER_BINARY_CASE(cmp_gt_f64, double, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_f64_k, double, Cmp_gt_fn)
      // cmp_ge
      REGISTER_BINARY_CASE(cmp_ge_i32, std::int32_t, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_i32_k, std::int32_t, Cmp_ge_fn)
      IMMEDIATE_BINARY_CASE(cmp_ge_i32_i, std::int32_t, Cmp_ge_fn)
      REGISTER_BINARY_CASE(cmp_ge_i64, std::int64_t, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_i64_k, std::int64_t, Cmp_ge_fn)
      IMMEDIATE_BINARY_CASE(cmp_ge_i64_i, std::int64_t, Cmp_ge_fn)
      REGISTER_BINARY_CASE(cmp_ge_f32, float, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_f32_k, float, Cmp_ge_fn)
      REGISTER_BINARY_CASE(cmp_ge_f64, double, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_f64_k, double, Cmp_ge_fn)
    default:
      throw std::runtime_error{"unimplemented bytecode opcode"};

#undef REGISTER_BINARY_CASE
#undef CONSTANT_BINARY_CASE
#undef IMMEDIATE_BINARY_CASE
#undef UNARY_CASE
    }
  }

  void Virtual_machine::wide_dispatch(bytecode::Opcode opcode)
  {
    switch (opcode)
    {
    case bytecode::Opcode::jmp_i:
      run_jmp_i(instruction_pointer, Operand_width::wide);
      break;
    case bytecode::Opcode::call_i:
      run_call_i(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::jnz_i:
      run_jnz_i(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::mov:
      run_mov(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::mov_i:
      run_mov_i(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::load_8:
      run_load<1>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::load_16:
      run_load<2>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::load_32:
      run_load<4>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::load_64:
      run_load<8>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::store_8:
      run_store<1>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::store_16:
      run_store<2>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::store_32:
      run_store<4>(instruction_pointer, *this, Operand_width::wide);
      break;
    case bytecode::Opcode::store_64:
      run_store<8>(instruction_pointer, *this, Operand_width::wide);
      break;

#define UNARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:     \
    run_register_unary<type, fn>(    \
      instruction_pointer,           \
      *this,                         \
      Operand_width::wide            \
    );                               \
    break;
#define REGISTER_BINARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:               \
    run_register_binary<type, fn>(             \
      instruction_pointer,                     \
      *this,                                   \
      Operand_width::wide                      \
    );                                         \
    break;
#define CONSTANT_BINARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:               \
    run_constant_binary<type, fn>(             \
      instruction_pointer,                     \
      *this,                                   \
      Operand_width::wide                      \
    );                                         \
    break;
#define IMMEDIATE_BINARY_CASE(opcode, type, fn) \
  case bytecode::Opcode::opcode:                \
    run_immediate_binary<type, fn>(             \
      instruction_pointer,                      \
      *this,                                    \
      Operand_width::wide                       \
    );                                          \
    break;

      // sx
      UNARY_CASE(sx_8, std::int8_t, Sx_fn)
      UNARY_CASE(sx_16, std::int16_t, Sx_fn)
      UNARY_CASE(sx_32, std::int32_t, Sx_fn)
      // neg
      UNARY_CASE(neg_i32, std::int32_t, Neg_fn)
      UNARY_CASE(neg_i64, std::int64_t, Neg_fn)
      UNARY_CASE(neg_f32, float, Neg_fn)
      UNARY_CASE(neg_f64, double, Neg_fn)
      // add
      REGISTER_BINARY_CASE(add_i32, std::int32_t, Add_fn)
      CONSTANT_BINARY_CASE(add_i32_k, std::int32_t, Add_fn)
      IMMEDIATE_BINARY_CASE(add_i32_i, std::int32_t, Add_fn)
      REGISTER_BINARY_CASE(add_i64, std::int64_t, Add_fn)
      CONSTANT_BINARY_CASE(add_i64_k, std::int64_t, Add_fn)
      IMMEDIATE_BINARY_CASE(add_i64_i, std::int64_t, Add_fn)
      REGISTER_BINARY_CASE(add_f32, float, Add_fn)
      CONSTANT_BINARY_CASE(add_f32_k, float, Add_fn)
      REGISTER_BINARY_CASE(add_f64, double, Add_fn)
      CONSTANT_BINARY_CASE(add_f64_k, double, Add_fn)
      // sub
      REGISTER_BINARY_CASE(sub_i32, std::int32_t, Sub_fn)
      CONSTANT_BINARY_CASE(sub_i32_k, std::int32_t, Sub_fn)
      IMMEDIATE_BINARY_CASE(sub_i32_i, std::int32_t, Sub_fn)
      REGISTER_BINARY_CASE(sub_i64, std::int64_t, Sub_fn)
      CONSTANT_BINARY_CASE(sub_i64_k, std::int64_t, Sub_fn)
      IMMEDIATE_BINARY_CASE(sub_i64_i, std::int64_t, Sub_fn)
      REGISTER_BINARY_CASE(sub_f32, float, Sub_fn)
      CONSTANT_BINARY_CASE(sub_f32_k, float, Sub_fn)
      REGISTER_BINARY_CASE(sub_f64, double, Sub_fn)
      CONSTANT_BINARY_CASE(sub_f64_k, double, Sub_fn)
      // mul
      REGISTER_BINARY_CASE(mul_i32, std::int32_t, Mul_fn)
      CONSTANT_BINARY_CASE(mul_i32_k, std::int32_t, Mul_fn)
      IMMEDIATE_BINARY_CASE(mul_i32_i, std::int32_t, Mul_fn)
      REGISTER_BINARY_CASE(mul_i64, std::int64_t, Mul_fn)
      CONSTANT_BINARY_CASE(mul_i64_k, std::int64_t, Mul_fn)
      IMMEDIATE_BINARY_CASE(mul_i64_i, std::int64_t, Mul_fn)
      REGISTER_BINARY_CASE(mul_f32, float, Mul_fn)
      CONSTANT_BINARY_CASE(mul_f32_k, float, Mul_fn)
      REGISTER_BINARY_CASE(mul_f64, double, Mul_fn)
      CONSTANT_BINARY_CASE(mul_f64_k, double, Mul_fn)
      // div
      REGISTER_BINARY_CASE(div_i32, std::int32_t, Div_fn)
      CONSTANT_BINARY_CASE(div_i32_k, std::int32_t, Div_fn)
      IMMEDIATE_BINARY_CASE(div_i32_i, std::int32_t, Div_fn)
      REGISTER_BINARY_CASE(div_i64, std::int64_t, Div_fn)
      CONSTANT_BINARY_CASE(div_i64_k, std::int64_t, Div_fn)
      IMMEDIATE_BINARY_CASE(div_i64_i, std::int64_t, Div_fn)
      REGISTER_BINARY_CASE(div_f32, float, Div_fn)
      CONSTANT_BINARY_CASE(div_f32_k, float, Div_fn)
      REGISTER_BINARY_CASE(div_f64, double, Div_fn)
      CONSTANT_BINARY_CASE(div_f64_k, double, Div_fn)
      // mod
      REGISTER_BINARY_CASE(mod_i32, std::int32_t, Mod_fn)
      CONSTANT_BINARY_CASE(mod_i32_k, std::int32_t, Mod_fn)
      IMMEDIATE_BINARY_CASE(mod_i32_i, std::int32_t, Mod_fn)
      REGISTER_BINARY_CASE(mod_i64, std::int64_t, Mod_fn)
      CONSTANT_BINARY_CASE(mod_i64_k, std::int64_t, Mod_fn)
      IMMEDIATE_BINARY_CASE(mod_i64_i, std::int64_t, Mod_fn)
      // cmp_eq
      REGISTER_BINARY_CASE(cmp_eq_i32, std::int32_t, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_i32_k, std::int32_t, Cmp_eq_fn)
      IMMEDIATE_BINARY_CASE(cmp_eq_i32_i, std::int32_t, Cmp_eq_fn)
      REGISTER_BINARY_CASE(cmp_eq_i64, std::int64_t, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_i64_k, std::int64_t, Cmp_eq_fn)
      IMMEDIATE_BINARY_CASE(cmp_eq_i64_i, std::int64_t, Cmp_eq_fn)
      REGISTER_BINARY_CASE(cmp_eq_f32, float, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_f32_k, float, Cmp_eq_fn)
      REGISTER_BINARY_CASE(cmp_eq_f64, double, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_f64_k, double, Cmp_eq_fn)
      // cmp_ne
      REGISTER_BINARY_CASE(cmp_ne_i32, std::int32_t, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_i32_k, std::int32_t, Cmp_ne_fn)
      IMMEDIATE_BINARY_CASE(cmp_ne_i32_i, std::int32_t, Cmp_ne_fn)
      REGISTER_BINARY_CASE(cmp_ne_i64, std::int64_t, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_i64_k, std::int64_t, Cmp_ne_fn)
      IMMEDIATE_BINARY_CASE(cmp_ne_i64_i, std::int64_t, Cmp_ne_fn)
      REGISTER_BINARY_CASE(cmp_ne_f32, float, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_f32_k, float, Cmp_ne_fn)
      REGISTER_BINARY_CASE(cmp_ne_f64, double, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_f64_k, double, Cmp_ne_fn)
      // cmp_lt
      REGISTER_BINARY_CASE(cmp_lt_i32, std::int32_t, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_i32_k, std::int32_t, Cmp_lt_fn)
      IMMEDIATE_BINARY_CASE(cmp_lt_i32_i, std::int32_t, Cmp_lt_fn)
      REGISTER_BINARY_CASE(cmp_lt_i64, std::int64_t, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_i64_k, std::int64_t, Cmp_lt_fn)
      IMMEDIATE_BINARY_CASE(cmp_lt_i64_i, std::int64_t, Cmp_lt_fn)
      REGISTER_BINARY_CASE(cmp_lt_f32, float, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_f32_k, float, Cmp_lt_fn)
      REGISTER_BINARY_CASE(cmp_lt_f64, double, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_f64_k, double, Cmp_lt_fn)
      // cmp_le
      REGISTER_BINARY_CASE(cmp_le_i32, std::int32_t, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_i32_k, std::int32_t, Cmp_le_fn)
      IMMEDIATE_BINARY_CASE(cmp_le_i32_i, std::int32_t, Cmp_le_fn)
      REGISTER_BINARY_CASE(cmp_le_i64, std::int64_t, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_i64_k, std::int64_t, Cmp_le_fn)
      IMMEDIATE_BINARY_CASE(cmp_le_i64_i, std::int64_t, Cmp_le_fn)
      REGISTER_BINARY_CASE(cmp_le_f32, float, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_f32_k, float, Cmp_le_fn)
      REGISTER_BINARY_CASE(cmp_le_f64, double, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_f64_k, double, Cmp_le_fn)
      // cmp_gt
      REGISTER_BINARY_CASE(cmp_gt_i32, std::int32_t, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_i32_k, std::int32_t, Cmp_gt_fn)
      IMMEDIATE_BINARY_CASE(cmp_gt_i32_i, std::int32_t, Cmp_gt_fn)
      REGISTER_BINARY_CASE(cmp_gt_i64, std::int64_t, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_i64_k, std::int64_t, Cmp_gt_fn)
      IMMEDIATE_BINARY_CASE(cmp_gt_i64_i, std::int64_t, Cmp_gt_fn)
      REGISTER_BINARY_CASE(cmp_gt_f32, float, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_f32_k, float, Cmp_gt_fn)
      REGISTER_BINARY_CASE(cmp_gt_f64, double, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_f64_k, double, Cmp_gt_fn)
      // cmp_ge
      REGISTER_BINARY_CASE(cmp_ge_i32, std::int32_t, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_i32_k, std::int32_t, Cmp_ge_fn)
      IMMEDIATE_BINARY_CASE(cmp_ge_i32_i, std::int32_t, Cmp_ge_fn)
      REGISTER_BINARY_CASE(cmp_ge_i64, std::int64_t, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_i64_k, std::int64_t, Cmp_ge_fn)
      IMMEDIATE_BINARY_CASE(cmp_ge_i64_i, std::int64_t, Cmp_ge_fn)
      REGISTER_BINARY_CASE(cmp_ge_f32, float, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_f32_k, float, Cmp_ge_fn)
      REGISTER_BINARY_CASE(cmp_ge_f64, double, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_f64_k, double, Cmp_ge_fn)
    default:
      throw std::runtime_error{"unimplemented wide bytecode opcode"};

#undef REGISTER_BINARY_CASE
#undef CONSTANT_BINARY_CASE
#undef IMMEDIATE_BINARY_CASE
#undef UNARY_CASE
    }
  }

} // namespace benson
