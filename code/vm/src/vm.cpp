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

    template <Operand_width width>
    bytecode::Register read_register(std::byte const *&instruction_pointer)
    {
      if constexpr (width == Operand_width::wide)
      {
        auto value = bytecode::Register::Underlying_type{};
        std::memcpy(&value, instruction_pointer, 2);
        instruction_pointer += 2;
        return bytecode::Register{value};
      }
      else
      {
        return bytecode::Register{
          static_cast<std::uint8_t>(*instruction_pointer++)
        };
      }
    }

    template <Operand_width width>
    bytecode::Constant read_constant(std::byte const *&instruction_pointer)
    {
      if constexpr (width == Operand_width::wide)
      {
        auto value = bytecode::Constant::Underlying_type{};
        std::memcpy(&value, instruction_pointer, 2);
        instruction_pointer += 2;
        return bytecode::Constant{value};
      }
      else
      {
        return bytecode::Constant{
          static_cast<std::uint8_t>(*instruction_pointer++)
        };
      }
    }

    template <Operand_width width>
    bytecode::Immediate read_immediate(std::byte const *&instruction_pointer)
    {
      if constexpr (width == Operand_width::wide)
      {
        auto value = bytecode::Immediate::Underlying_type{};
        std::memcpy(&value, instruction_pointer, 2);
        instruction_pointer += 2;
        return bytecode::Immediate{value};
      }
      else
      {
        return bytecode::Immediate{
          static_cast<std::int8_t>(*instruction_pointer++)
        };
      }
    }

    template <Operand_width width>
    void run_lookup_k(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const k = read_constant<width>(instruction_pointer);
      vm.set_register_value(dst, vm.lookup_constant(k));
    }

    template <Operand_width width>
    void run_mov(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const src = read_register<width>(instruction_pointer);
      (*vm.registers)[dst.value] = (*vm.registers)[src.value];
    }

    template <Operand_width width>
    void run_mov_i(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const src = read_immediate<width>(instruction_pointer);
      vm.set_register_value(dst, src.value);
    }

    template <Operand_width width, typename CppType, typename Fn>
    void run_register_unary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const src = read_register<width>(instruction_pointer);
      vm.set_register_value(dst, Fn{}(vm.get_register_value<CppType>(src)));
    }

    template <Operand_width width, typename OperandType, typename Fn>
    void run_register_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const lhs = read_register<width>(instruction_pointer);
      auto const rhs = read_register<width>(instruction_pointer);
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          vm.get_register_value<OperandType>(rhs)
        )
      );
    }

    template <Operand_width width, typename OperandType, typename Fn>
    void run_constant_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const lhs = read_register<width>(instruction_pointer);
      auto const rhs = read_constant<width>(instruction_pointer);
      vm.set_register_value(
        dst,
        Fn{}(
          vm.get_register_value<OperandType>(lhs),
          vm.get_constant_value<OperandType>(rhs)
        )
      );
    }

    template <Operand_width width, typename OperandType, typename Fn>
    void run_immediate_binary(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const lhs = read_register<width>(instruction_pointer);
      auto const rhs = read_immediate<width>(instruction_pointer);
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

    template <Operand_width width, std::size_t N>
    void run_load(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const base = read_register<width>(instruction_pointer);
      auto const offset = read_immediate<width>(instruction_pointer);

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

    template <Operand_width width, std::size_t N>
    void run_store(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const src = read_register<width>(instruction_pointer);
      auto const base = read_register<width>(instruction_pointer);
      auto const offset = read_immediate<width>(instruction_pointer);

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

    template <Operand_width width>
    void run_jmp_i(std::byte const *&instruction_pointer)
    {
      auto const offset = read_immediate<width>(instruction_pointer);
      instruction_pointer += offset.value;
    }

    void push_bytes(Virtual_machine &vm, std::span<std::byte const> bytes)
    {
      auto const sp = vm.get_register_value<Pointer>(bytecode::sp).decode();
      if (sp.space != Address_space::stack)
      {
        throw std::runtime_error{"unsupported address space for call stack"};
      }
      auto const new_offset = sp.offset - bytes.size();
      std::memcpy(vm.stack->data() + new_offset, bytes.data(), bytes.size());
      vm.set_register_value(
        bytecode::sp,
        Pointer{Address_space::stack, new_offset}
      );
    }

    void push_u64(Virtual_machine &vm, std::uint64_t value)
    {
      push_bytes(vm, std::as_bytes(std::span{&value, std::size_t{1}}));
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

    template <Operand_width width>
    void run_call_i(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const offset = read_immediate<width>(instruction_pointer);
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

    template <Operand_width width>
    void run_jnz_i(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const src = read_register<width>(instruction_pointer);
      auto const offset = read_immediate<width>(instruction_pointer);
      if (vm.get_register_value<std::int32_t>(src) != 0)
      {
        instruction_pointer += offset.value;
      }
    }

    template <typename T>
    void push_value(Virtual_machine &vm, T value)
    {
      push_bytes(vm, std::as_bytes(std::span{&value, std::size_t{1}}));
    }

    void push_arg(
      Virtual_machine &vm,
      ir::Type const *type,
      ir::Constant_value const &value
    )
    {
      auto const require = [&]<typename T>() -> T {
        auto const *typed = std::get_if<T>(&value);
        if (!typed)
        {
          throw std::runtime_error{"argument type mismatch"};
        }
        return *typed;
      };
      std::visit(
        [&]<typename T>(T const &) {
          if constexpr (std::same_as<T, ir::Int8_type>)
          {
            push_value(vm, require.template operator()<std::int8_t>());
          }
          else if constexpr (std::same_as<T, ir::Int16_type>)
          {
            push_value(vm, require.template operator()<std::int16_t>());
          }
          else if constexpr (std::same_as<T, ir::Int32_type>)
          {
            push_value(vm, require.template operator()<std::int32_t>());
          }
          else if constexpr (std::same_as<T, ir::Int64_type>)
          {
            push_value(vm, require.template operator()<std::int64_t>());
          }
          else if constexpr (std::same_as<T, ir::Float32_type>)
          {
            push_value(vm, require.template operator()<float>());
          }
          else if constexpr (std::same_as<T, ir::Float64_type>)
          {
            push_value(vm, require.template operator()<double>());
          }
          else if constexpr (std::same_as<T, ir::Bool_type>)
          {
            push_value(vm, std::uint8_t{require.template operator()<bool>()});
          }
          else
          {
            throw std::runtime_error{"unsupported argument type"};
          }
        },
        type->data
      );
    }

    ir::Constant_value
    decode_return(Virtual_machine const &vm, ir::Type const *type)
    {
      return std::visit(
        [&]<typename T>(T const &) -> ir::Constant_value {
          if constexpr (std::same_as<T, ir::Int8_type>)
          {
            return vm.get_register_value<std::int8_t>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Int16_type>)
          {
            return vm.get_register_value<std::int16_t>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Int32_type>)
          {
            return vm.get_register_value<std::int32_t>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Int64_type>)
          {
            return vm.get_register_value<std::int64_t>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Float32_type>)
          {
            return vm.get_register_value<float>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Float64_type>)
          {
            return vm.get_register_value<double>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Bool_type>)
          {
            return vm.get_register_value<bool>(bytecode::gpr(1));
          }
          else if constexpr (std::same_as<T, ir::Void_type>)
          {
            return ir::Void_value{};
          }
          else
          {
            throw std::runtime_error{"unsupported return type"};
          }
        },
        type->data
      );
    }

    constexpr std::byte halt_byte{static_cast<std::byte>(bytecode::Opcode::exit)
    };

  } // namespace

  Virtual_machine::Virtual_machine()
      : instruction_pointer{nullptr},
        constant_memory{nullptr},
        constant_table{nullptr},
        registers{std::make_unique<std::array<std::uint64_t, 64 * 1024>>()},
        stack{std::make_unique<std::array<std::byte, 16 * 1024 * 1024>>()}
  {
    set_register_value(
      bytecode::sp,
      Pointer{Address_space::stack, stack->size()}
    );
  }

  void Virtual_machine::load(bytecode::Module const &m)
  {
    module = &m;
    instruction_pointer = m.code.data();
    constant_memory = m.constant_data.data();
    constant_table = m.constant_table.data();
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

  ir::Constant_value Virtual_machine::call(
    Spelling name,
    std::span<ir::Constant_value const> args
  )
  {
    if (!module)
    {
      throw std::runtime_error{"no module loaded"};
    }
    auto const it = module->functions.find(name);
    if (it == module->functions.end())
    {
      throw std::runtime_error{"unknown function"};
    }
    auto const &fn = it->second;
    if (args.size() != fn.parameter_types.size())
    {
      throw std::runtime_error{"argument count mismatch"};
    }
    auto const saved_ip = instruction_pointer;
    set_register_value(
      bytecode::sp,
      Pointer{Address_space::stack, stack->size()}
    );
    for (auto i = std::size_t{}; i < args.size(); ++i)
    {
      push_arg(*this, fn.parameter_types[i], args[i]);
    }
    push_u64(
      *this,
      static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(&halt_byte))
    );
    instruction_pointer = module->code.data() + fn.position;
    run();
    instruction_pointer = saved_ip;
    return decode_return(*this, fn.return_type);
  }

  void Virtual_machine::dispatch(bytecode::Opcode opcode)
  {
    switch (opcode)
    {
    case bytecode::Opcode::wide:
      wide_dispatch(static_cast<bytecode::Opcode>(*instruction_pointer++));
      break;
    case bytecode::Opcode::jmp_i:
      run_jmp_i<Operand_width::narrow>(instruction_pointer);
      break;
    case bytecode::Opcode::call_i:
      run_call_i<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::ret:
      run_ret(instruction_pointer, *this);
      break;
    case bytecode::Opcode::jnz_i:
      run_jnz_i<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::mov:
      run_mov<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::mov_i:
      run_mov_i<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_8:
      run_load<Operand_width::narrow, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_16:
      run_load<Operand_width::narrow, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_32:
      run_load<Operand_width::narrow, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_64:
      run_load<Operand_width::narrow, 8>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_8:
      run_store<Operand_width::narrow, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_16:
      run_store<Operand_width::narrow, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_32:
      run_store<Operand_width::narrow, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_64:
      run_store<Operand_width::narrow, 8>(instruction_pointer, *this);
      break;

#define UNARY_CASE(opcode, type, fn)                     \
  case bytecode::Opcode::opcode:                         \
    run_register_unary<Operand_width::narrow, type, fn>( \
      instruction_pointer,                               \
      *this                                              \
    );                                                   \
    break;
#define REGISTER_BINARY_CASE(opcode, type, fn)            \
  case bytecode::Opcode::opcode:                          \
    run_register_binary<Operand_width::narrow, type, fn>( \
      instruction_pointer,                                \
      *this                                               \
    );                                                    \
    break;
#define CONSTANT_BINARY_CASE(opcode, type, fn)            \
  case bytecode::Opcode::opcode:                          \
    run_constant_binary<Operand_width::narrow, type, fn>( \
      instruction_pointer,                                \
      *this                                               \
    );                                                    \
    break;
#define IMMEDIATE_BINARY_CASE(opcode, type, fn)            \
  case bytecode::Opcode::opcode:                           \
    run_immediate_binary<Operand_width::narrow, type, fn>( \
      instruction_pointer,                                 \
      *this                                                \
    );                                                     \
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
      run_jmp_i<Operand_width::wide>(instruction_pointer);
      break;
    case bytecode::Opcode::call_i:
      run_call_i<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::jnz_i:
      run_jnz_i<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::lookup_k:
      run_lookup_k<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::mov:
      run_mov<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::mov_i:
      run_mov_i<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_8:
      run_load<Operand_width::wide, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_16:
      run_load<Operand_width::wide, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_32:
      run_load<Operand_width::wide, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_64:
      run_load<Operand_width::wide, 8>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_8:
      run_store<Operand_width::wide, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_16:
      run_store<Operand_width::wide, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_32:
      run_store<Operand_width::wide, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_64:
      run_store<Operand_width::wide, 8>(instruction_pointer, *this);
      break;

#define UNARY_CASE(opcode, type, fn)                   \
  case bytecode::Opcode::opcode:                       \
    run_register_unary<Operand_width::wide, type, fn>( \
      instruction_pointer,                             \
      *this                                            \
    );                                                 \
    break;
#define REGISTER_BINARY_CASE(opcode, type, fn)          \
  case bytecode::Opcode::opcode:                        \
    run_register_binary<Operand_width::wide, type, fn>( \
      instruction_pointer,                              \
      *this                                             \
    );                                                  \
    break;
#define CONSTANT_BINARY_CASE(opcode, type, fn)          \
  case bytecode::Opcode::opcode:                        \
    run_constant_binary<Operand_width::wide, type, fn>( \
      instruction_pointer,                              \
      *this                                             \
    );                                                  \
    break;
#define IMMEDIATE_BINARY_CASE(opcode, type, fn)          \
  case bytecode::Opcode::opcode:                         \
    run_immediate_binary<Operand_width::wide, type, fn>( \
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
      throw std::runtime_error{"unimplemented wide bytecode opcode"};

#undef REGISTER_BINARY_CASE
#undef CONSTANT_BINARY_CASE
#undef IMMEDIATE_BINARY_CASE
#undef UNARY_CASE
    }
  }

} // namespace benson
