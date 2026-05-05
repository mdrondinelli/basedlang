#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "bytecode/immediate.h"
#include "vm/vm.h"

namespace benson::vm
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
      vm.registers[vm.frame_base + dst.value] =
        vm.registers[vm.frame_base + src.value];
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
          return vm.module->constant_data.data();
        case Address_space::stack:
          return vm.stack->data();
        case Address_space::heap:
          // TODO: heap load implementation
        default:
          throw std::runtime_error{"unsupported address space for load"};
        }
      }();

      auto value = std::uint64_t{};
      // TODO: safety bounds check
      std::memcpy(&value, space_pointer + base_address + offset.value, N);
      vm.registers[vm.frame_base + dst.value] = value;
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
      // TODO: safety bounds check
      std::memcpy(
        vm.stack->data() + base_address + offset.value,
        &vm.registers[vm.frame_base + src.value],
        N
      );
    }

    template <Operand_width width, std::size_t N>
    void run_load_sp(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const offset = read_immediate<width>(instruction_pointer);
      auto value = std::uint64_t{};
      // TODO: safety bounds check
      std::memcpy(
        &value,
        vm.stack->data() + vm.stack_pointer + offset.value,
        N
      );
      vm.registers[vm.frame_base + dst.value] = value;
    }

    template <Operand_width width, std::size_t N>
    void run_store_sp(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const src = read_register<width>(instruction_pointer);
      auto const offset = read_immediate<width>(instruction_pointer);
      // TODO: safety bounds check
      std::memcpy(
        vm.stack->data() + vm.stack_pointer + offset.value,
        &vm.registers[vm.frame_base + src.value],
        N
      );
    }

    template <Operand_width width>
    void run_push_sp_i(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const amount = read_immediate<width>(instruction_pointer);
      assert(amount.value >= 0);
      vm.stack_pointer -= static_cast<std::uint64_t>(amount.value);
    }

    template <Operand_width width>
    void run_push_sp(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const amount = read_register<width>(instruction_pointer);
      // TODO: validate non-negative amount when running unverified bytecode.
      vm.stack_pointer -=
        static_cast<std::uint64_t>(vm.get_register_value<std::int64_t>(amount));
    }

    template <Operand_width width>
    void run_mov_sp_i(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const dst = read_register<width>(instruction_pointer);
      auto const offset = read_immediate<width>(instruction_pointer);
      vm.set_register_value(
        dst,
        Pointer{
          Address_space::stack,
          static_cast<std::uint64_t>(
            static_cast<std::int64_t>(vm.stack_pointer) + offset.value
          )
        }
      );
    }

    template <Operand_width width>
    void run_jmp_i(std::byte const *&instruction_pointer)
    {
      auto const offset = read_immediate<width>(instruction_pointer);
      instruction_pointer += offset.value;
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

    template <Operand_width width, bool has_return>
    void run_call_i(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const function_index = read_immediate<width>(instruction_pointer);
      assert(function_index.value >= 0);
      auto const base = read_register<width>(instruction_pointer);
      auto destination = std::optional<std::size_t>{};
      if constexpr (has_return)
      {
        auto const dst = read_register<width>(instruction_pointer);
        destination = vm.frame_base + dst.value;
      }
      auto const new_frame_base = vm.frame_base + base.value;
      auto const &function =
        vm.module->functions[static_cast<std::size_t>(function_index.value)];
      vm.call_stack.push_back(
        Virtual_machine::Call_frame{
          .return_address = instruction_pointer,
          .frame_base = vm.frame_base,
          .stack_pointer = vm.stack_pointer,
          .destination = destination,
        }
      );
      vm.frame_base = new_frame_base;
      vm.registers.resize(
        std::max(
          vm.registers.size(),
          new_frame_base + static_cast<std::size_t>(function.register_count)
        )
      );
      instruction_pointer = vm.module->code.data() + function.position;
    }

    template <Operand_width width>
    void run_ret(std::byte const *&instruction_pointer, Virtual_machine &vm)
    {
      auto const src = read_register<width>(instruction_pointer);
      auto frame = vm.call_stack.back();
      vm.call_stack.pop_back();
      assert(frame.destination);
      auto const value = vm.registers[vm.frame_base + src.value];
      vm.frame_base = frame.frame_base;
      vm.stack_pointer = frame.stack_pointer;
      vm.registers[*frame.destination] = value;
      instruction_pointer = frame.return_address;
    }

    void run_ret_void(
      std::byte const *&instruction_pointer,
      Virtual_machine &vm
    )
    {
      auto const frame = vm.call_stack.back();
      vm.call_stack.pop_back();
      vm.frame_base = frame.frame_base;
      vm.stack_pointer = frame.stack_pointer;
      instruction_pointer = frame.return_address;
    }

    Scalar decode_return(Virtual_machine const &vm, bytecode::Scalar_type type)
    {
      switch (type)
      {
      case bytecode::Scalar_type::int8:
        return vm.get_register_value<std::int8_t>(bytecode::gpr(0));
      case bytecode::Scalar_type::int16:
        return vm.get_register_value<std::int16_t>(bytecode::gpr(0));
      case bytecode::Scalar_type::int32:
        return vm.get_register_value<std::int32_t>(bytecode::gpr(0));
      case bytecode::Scalar_type::int64:
        return vm.get_register_value<std::int64_t>(bytecode::gpr(0));
      case bytecode::Scalar_type::float_:
        return vm.get_register_value<float>(bytecode::gpr(0));
      case bytecode::Scalar_type::double_:
        return vm.get_register_value<double>(bytecode::gpr(0));
      case bytecode::Scalar_type::bool_:
        return vm.get_register_value<bool>(bytecode::gpr(0));
      case bytecode::Scalar_type::void_:
        return Scalar::void_;
      }
      throw Virtual_machine::Unsupported_return_type_error{type};
    }

  } // namespace

  Virtual_machine::Virtual_machine()
      : instruction_pointer{nullptr},
        registers{},
        stack{std::make_unique<std::array<std::byte, 16 * 1024 * 1024>>()}
  {
    stack_pointer = stack->size();
  }

  void Virtual_machine::load(bytecode::Module const &m)
  {
    module = &m;
    instruction_pointer = m.code.data();
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

  Scalar Virtual_machine::call(Spelling name, std::span<Scalar const> args)
  {
    assert(module != nullptr);
    auto const it = module->function_indices.find(name);
    if (it == module->function_indices.end())
    {
      throw Unknown_function_error{name};
    }
    auto const &fn = module->functions[it->second];
    if (args.size() != fn.parameter_types.size())
    {
      throw Argument_count_error{
        static_cast<std::ptrdiff_t>(fn.parameter_types.size()),
        static_cast<std::ptrdiff_t>(args.size()),
      };
    }
    for (auto const &arg : args)
    {
      auto const i = &arg - &args[0];
      auto const param_type = fn.parameter_types[i];
      if (arg.type() != param_type)
      {
        throw Virtual_machine::Argument_type_error{i, param_type};
      }
    }
    auto const ip = instruction_pointer;
    auto const old_frame_base = frame_base;
    auto const old_stack_pointer = stack_pointer;
    auto const old_call_stack_size = call_stack.size();
    frame_base = 0;
    registers.resize(
      std::max<std::size_t>(
        registers.size(),
        std::max<std::size_t>(
          static_cast<std::size_t>(fn.register_count),
          args.size() + 1
        )
      )
    );
    for (auto const &arg : args)
    {
      auto const i = &arg - &args[0];
      switch (arg.type())
      {
      case bytecode::Scalar_type::int8:
        set_register_value(bytecode::Register{i}, arg.as<std::int8_t>());
        break;
      case bytecode::Scalar_type::int16:
        set_register_value(bytecode::Register{i}, arg.as<std::int16_t>());
        break;
      case bytecode::Scalar_type::int32:
        set_register_value(bytecode::Register{i}, arg.as<std::int32_t>());
        break;
      case bytecode::Scalar_type::int64:
        set_register_value(bytecode::Register{i}, arg.as<std::int64_t>());
        break;
      case bytecode::Scalar_type::float_:
        set_register_value(bytecode::Register{i}, arg.as<float>());
        break;
      case bytecode::Scalar_type::double_:
        set_register_value(bytecode::Register{i}, arg.as<double>());
        break;
      case bytecode::Scalar_type::bool_:
        set_register_value(bytecode::Register{i}, arg.as<bool>());
        break;
      case bytecode::Scalar_type::void_:
        throw Unsupported_argument_type_error{i, arg.type()};
      }
    }
    auto const exit_byte = bytecode::Opcode::exit;
    call_stack.push_back(
      Call_frame{
        .return_address = reinterpret_cast<std::byte const *>(&exit_byte),
        .frame_base = old_frame_base,
        .stack_pointer = old_stack_pointer,
        .destination = fn.return_type == bytecode::Scalar_type::void_
                         ? std::nullopt
                         : std::optional<std::size_t>{0},
      }
    );
    instruction_pointer = module->code.data() + fn.position;
    run();
    call_stack.resize(old_call_stack_size);
    instruction_pointer = ip;
    frame_base = old_frame_base;
    stack_pointer = old_stack_pointer;
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
      run_call_i<Operand_width::narrow, true>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::call_void_i:
      run_call_i<Operand_width::narrow, false>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::ret:
      run_ret<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::ret_void:
      run_ret_void(instruction_pointer, *this);
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
    case bytecode::Opcode::load_sp_8:
      run_load_sp<Operand_width::narrow, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_sp_16:
      run_load_sp<Operand_width::narrow, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_sp_32:
      run_load_sp<Operand_width::narrow, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_sp_64:
      run_load_sp<Operand_width::narrow, 8>(instruction_pointer, *this);
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
    case bytecode::Opcode::store_sp_8:
      run_store_sp<Operand_width::narrow, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_sp_16:
      run_store_sp<Operand_width::narrow, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_sp_32:
      run_store_sp<Operand_width::narrow, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_sp_64:
      run_store_sp<Operand_width::narrow, 8>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::push_sp_i:
      run_push_sp_i<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::push_sp:
      run_push_sp<Operand_width::narrow>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::mov_sp_i:
      run_mov_sp_i<Operand_width::narrow>(instruction_pointer, *this);
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
      run_call_i<Operand_width::wide, true>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::call_void_i:
      run_call_i<Operand_width::wide, false>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::ret:
      run_ret<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::ret_void:
      run_ret_void(instruction_pointer, *this);
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
    case bytecode::Opcode::load_sp_8:
      run_load_sp<Operand_width::wide, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_sp_16:
      run_load_sp<Operand_width::wide, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_sp_32:
      run_load_sp<Operand_width::wide, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::load_sp_64:
      run_load_sp<Operand_width::wide, 8>(instruction_pointer, *this);
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
    case bytecode::Opcode::store_sp_8:
      run_store_sp<Operand_width::wide, 1>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_sp_16:
      run_store_sp<Operand_width::wide, 2>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_sp_32:
      run_store_sp<Operand_width::wide, 4>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::store_sp_64:
      run_store_sp<Operand_width::wide, 8>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::push_sp_i:
      run_push_sp_i<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::push_sp:
      run_push_sp<Operand_width::wide>(instruction_pointer, *this);
      break;
    case bytecode::Opcode::mov_sp_i:
      run_mov_sp_i<Operand_width::wide>(instruction_pointer, *this);
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

} // namespace benson::vm
