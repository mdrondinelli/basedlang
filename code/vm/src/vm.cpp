#include <cstring>
#include <memory>
#include <stdexcept>

#include "bytecode/immediate.h"
#include "vm/vm.h"

namespace benson
{

  namespace
  {

    template <typename ConstantType>
    void run_lookup_k(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      static_assert(
        std::is_same_v<ConstantType, bytecode::Wide_constant> ||
        std::is_same_v<ConstantType, bytecode::Constant>
      );
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto k = ConstantType{};
      std::memcpy(&k, instruction_pointer, sizeof(k));
      instruction_pointer += sizeof(k);
      vm.set_register_value(dst, vm.lookup_constant(k));
    }

    template <typename CppType, typename Fn>
    void run_register_unary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const src = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      vm.set_register_value(dst, Fn{}(vm.get_register_value<CppType>(src)));
    }

    template <typename OperandType, typename Fn>
    void run_register_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const lhs = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const rhs = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          vm.get_register_value<OperandType>(rhs)
        )
      );
    }

    template <typename OperandType, typename ConstantType, typename Fn>
    void run_constant_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      static_assert(
        std::is_same_v<ConstantType, bytecode::Wide_constant> ||
        std::is_same_v<ConstantType, bytecode::Constant>
      );
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const lhs = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto rhs = ConstantType{};
      std::memcpy(&rhs, instruction_pointer, sizeof(rhs));
      instruction_pointer += sizeof(rhs);
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          vm.get_constant_value<OperandType>(rhs)
        )
      );
    }

    template <typename OperandType, typename ImmediateType, typename Fn>
    void run_immediate_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      static_assert(
        std::is_same_v<ImmediateType, bytecode::Wide_immediate> ||
        std::is_same_v<ImmediateType, bytecode::Immediate>
      );
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const lhs = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto rhs = ImmediateType{};
      std::memcpy(&rhs, instruction_pointer, sizeof(rhs));
      instruction_pointer += sizeof(rhs);
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

    template <std::size_t N, typename OffsetType>
    void run_load(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const base = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto offset = OffsetType{};
      std::memcpy(&offset, instruction_pointer, sizeof(offset));
      instruction_pointer += sizeof(offset);

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
      vm.registers[static_cast<std::size_t>(dst)] = value;
    }

    template <std::size_t N, typename OffsetType>
    void run_store(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const src = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto const base = static_cast<bytecode::Register>(
        static_cast<std::uint8_t>(*instruction_pointer++)
      );
      auto offset = OffsetType{};
      std::memcpy(&offset, instruction_pointer, sizeof(offset));
      instruction_pointer += sizeof(offset);

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
        &vm.registers[static_cast<std::size_t>(src)],
        N
      );
    }

    template <typename OffsetType>
    void run_jmp(std::byte const *&instruction_pointer)
    {
      auto offset = OffsetType{};
      std::memcpy(&offset, instruction_pointer, sizeof(offset));
      instruction_pointer += sizeof(offset);
      instruction_pointer += offset.value;
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
    case bytecode::Opcode::jmp_i:
      run_jmp<bytecode::Immediate>(instruction_pointer);
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k<bytecode::Constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_8:
      run_load<1, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_16:
      run_load<2, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_32:
      run_load<4, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_64:
      run_load<8, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_8:
      run_store<1, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_16:
      run_store<2, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_32:
      run_store<4, bytecode::Immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_64:
      run_store<8, bytecode::Immediate>(instruction_pointer, *this);
      break;

#define UNARY_CASE(opcode, type, fn)                          \
  case bytecode::Opcode::opcode:                              \
    run_register_unary<type, fn>(instruction_pointer, *this); \
    break;
#define REGISTER_BINARY_CASE(opcode, type, fn)                 \
  case bytecode::Opcode::opcode:                               \
    run_register_binary<type, fn>(instruction_pointer, *this); \
    break;
#define CONSTANT_BINARY_CASE(opcode, type, fn)         \
  case bytecode::Opcode::opcode:                       \
    run_constant_binary<type, bytecode::Constant, fn>( \
      instruction_pointer,                             \
      *this                                            \
    );                                                 \
    break;
#define IMMEDIATE_BINARY_CASE(opcode, type, fn)          \
  case bytecode::Opcode::opcode:                         \
    run_immediate_binary<type, bytecode::Immediate, fn>( \
      instruction_pointer,                               \
      *this                                              \
    );                                                   \
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
    }
  }

  void Virtual_machine::wide_dispatch(bytecode::Opcode opcode)
  {
    switch (opcode)
    {
    case bytecode::Opcode::jmp_i:
      run_jmp<bytecode::Wide_immediate>(instruction_pointer);
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k<bytecode::Wide_constant>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_8:
      run_load<1, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_16:
      run_load<2, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_32:
      run_load<4, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_64:
      run_load<8, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_8:
      run_store<1, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_16:
      run_store<2, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_32:
      run_store<4, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_64:
      run_store<8, bytecode::Wide_immediate>(instruction_pointer, *this);
      break;

#define CONSTANT_BINARY_CASE(opcode, type, fn)              \
  case bytecode::Opcode::opcode:                            \
    run_constant_binary<type, bytecode::Wide_constant, fn>( \
      instruction_pointer,                                  \
      *this                                                 \
    );                                                      \
    break;
#define IMMEDIATE_BINARY_CASE(opcode, type, fn)               \
  case bytecode::Opcode::opcode:                              \
    run_immediate_binary<type, bytecode::Wide_immediate, fn>( \
      instruction_pointer,                                    \
      *this                                                   \
    );                                                        \
    break;

      // add
      CONSTANT_BINARY_CASE(add_i32_k, std::int32_t, Add_fn)
      IMMEDIATE_BINARY_CASE(add_i32_i, std::int32_t, Add_fn)
      CONSTANT_BINARY_CASE(add_i64_k, std::int64_t, Add_fn)
      IMMEDIATE_BINARY_CASE(add_i64_i, std::int64_t, Add_fn)
      CONSTANT_BINARY_CASE(add_f32_k, float, Add_fn)
      CONSTANT_BINARY_CASE(add_f64_k, double, Add_fn)
      // sub
      CONSTANT_BINARY_CASE(sub_i32_k, std::int32_t, Sub_fn)
      IMMEDIATE_BINARY_CASE(sub_i32_i, std::int32_t, Sub_fn)
      CONSTANT_BINARY_CASE(sub_i64_k, std::int64_t, Sub_fn)
      IMMEDIATE_BINARY_CASE(sub_i64_i, std::int64_t, Sub_fn)
      CONSTANT_BINARY_CASE(sub_f32_k, float, Sub_fn)
      CONSTANT_BINARY_CASE(sub_f64_k, double, Sub_fn)
      // mul
      CONSTANT_BINARY_CASE(mul_i32_k, std::int32_t, Mul_fn)
      IMMEDIATE_BINARY_CASE(mul_i32_i, std::int32_t, Mul_fn)
      CONSTANT_BINARY_CASE(mul_i64_k, std::int64_t, Mul_fn)
      IMMEDIATE_BINARY_CASE(mul_i64_i, std::int64_t, Mul_fn)
      CONSTANT_BINARY_CASE(mul_f32_k, float, Mul_fn)
      CONSTANT_BINARY_CASE(mul_f64_k, double, Mul_fn)
      // div
      CONSTANT_BINARY_CASE(div_i32_k, std::int32_t, Div_fn)
      IMMEDIATE_BINARY_CASE(div_i32_i, std::int32_t, Div_fn)
      CONSTANT_BINARY_CASE(div_i64_k, std::int64_t, Div_fn)
      IMMEDIATE_BINARY_CASE(div_i64_i, std::int64_t, Div_fn)
      CONSTANT_BINARY_CASE(div_f32_k, float, Div_fn)
      CONSTANT_BINARY_CASE(div_f64_k, double, Div_fn)
      // mod
      CONSTANT_BINARY_CASE(mod_i32_k, std::int32_t, Mod_fn)
      IMMEDIATE_BINARY_CASE(mod_i32_i, std::int32_t, Mod_fn)
      CONSTANT_BINARY_CASE(mod_i64_k, std::int64_t, Mod_fn)
      IMMEDIATE_BINARY_CASE(mod_i64_i, std::int64_t, Mod_fn)
      // cmp_eq
      CONSTANT_BINARY_CASE(cmp_eq_i32_k, std::int32_t, Cmp_eq_fn)
      IMMEDIATE_BINARY_CASE(cmp_eq_i32_i, std::int32_t, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_i64_k, std::int64_t, Cmp_eq_fn)
      IMMEDIATE_BINARY_CASE(cmp_eq_i64_i, std::int64_t, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_f32_k, float, Cmp_eq_fn)
      CONSTANT_BINARY_CASE(cmp_eq_f64_k, double, Cmp_eq_fn)
      // cmp_ne
      CONSTANT_BINARY_CASE(cmp_ne_i32_k, std::int32_t, Cmp_ne_fn)
      IMMEDIATE_BINARY_CASE(cmp_ne_i32_i, std::int32_t, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_i64_k, std::int64_t, Cmp_ne_fn)
      IMMEDIATE_BINARY_CASE(cmp_ne_i64_i, std::int64_t, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_f32_k, float, Cmp_ne_fn)
      CONSTANT_BINARY_CASE(cmp_ne_f64_k, double, Cmp_ne_fn)
      // cmp_lt
      CONSTANT_BINARY_CASE(cmp_lt_i32_k, std::int32_t, Cmp_lt_fn)
      IMMEDIATE_BINARY_CASE(cmp_lt_i32_i, std::int32_t, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_i64_k, std::int64_t, Cmp_lt_fn)
      IMMEDIATE_BINARY_CASE(cmp_lt_i64_i, std::int64_t, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_f32_k, float, Cmp_lt_fn)
      CONSTANT_BINARY_CASE(cmp_lt_f64_k, double, Cmp_lt_fn)
      // cmp_le
      CONSTANT_BINARY_CASE(cmp_le_i32_k, std::int32_t, Cmp_le_fn)
      IMMEDIATE_BINARY_CASE(cmp_le_i32_i, std::int32_t, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_i64_k, std::int64_t, Cmp_le_fn)
      IMMEDIATE_BINARY_CASE(cmp_le_i64_i, std::int64_t, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_f32_k, float, Cmp_le_fn)
      CONSTANT_BINARY_CASE(cmp_le_f64_k, double, Cmp_le_fn)
      // cmp_gt
      CONSTANT_BINARY_CASE(cmp_gt_i32_k, std::int32_t, Cmp_gt_fn)
      IMMEDIATE_BINARY_CASE(cmp_gt_i32_i, std::int32_t, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_i64_k, std::int64_t, Cmp_gt_fn)
      IMMEDIATE_BINARY_CASE(cmp_gt_i64_i, std::int64_t, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_f32_k, float, Cmp_gt_fn)
      CONSTANT_BINARY_CASE(cmp_gt_f64_k, double, Cmp_gt_fn)
      // cmp_ge
      CONSTANT_BINARY_CASE(cmp_ge_i32_k, std::int32_t, Cmp_ge_fn)
      IMMEDIATE_BINARY_CASE(cmp_ge_i32_i, std::int32_t, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_i64_k, std::int64_t, Cmp_ge_fn)
      IMMEDIATE_BINARY_CASE(cmp_ge_i64_i, std::int64_t, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_f32_k, float, Cmp_ge_fn)
      CONSTANT_BINARY_CASE(cmp_ge_f64_k, double, Cmp_ge_fn)
    default:
      throw std::runtime_error{"unimplemented wide bytecode opcode"};

#undef CONSTANT_BINARY_CASE
#undef IMMEDIATE_BINARY_CASE
    }
  }

} // namespace benson
