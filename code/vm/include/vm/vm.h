#ifndef BENSON_VM_VM_H
#define BENSON_VM_VM_H

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>

#include "bytecode/constant.h"
#include "bytecode/module.h"
#include "bytecode/opcode.h"
#include "bytecode/register.h"
#include "spelling/spelling.h"

#include "pointer.h"
#include "scalar.h"

namespace benson::vm
{

  class Virtual_machine
  {
  public:
    class Call_error: public std::runtime_error
    {
    public:
      using std::runtime_error::runtime_error;
    };

    class Unknown_function_error: public Call_error
    {
    public:
      explicit Unknown_function_error(Spelling name)
          : Call_error{"unknown function"}, name{name}
      {
      }

      Spelling name;
    };

    class Argument_count_error: public Call_error
    {
    public:
      Argument_count_error(std::ptrdiff_t expected, std::ptrdiff_t actual)
          : Call_error{"argument count mismatch"},
            expected{expected},
            actual{actual}
      {
      }

      std::ptrdiff_t expected;
      std::ptrdiff_t actual;
    };

    class Argument_type_error: public Call_error
    {
    public:
      Argument_type_error(
        std::ptrdiff_t index,
        bytecode::Scalar_type expected_type
      )
          : Call_error{"argument type mismatch"},
            index{index},
            expected_type{expected_type}
      {
      }

      std::ptrdiff_t index;
      bytecode::Scalar_type expected_type;
    };

    class Unsupported_argument_type_error: public Call_error
    {
    public:
      Unsupported_argument_type_error(
        std::ptrdiff_t index,
        bytecode::Scalar_type type
      )
          : Call_error{"unsupported argument type"}, index{index}, type{type}
      {
      }

      std::ptrdiff_t index;
      bytecode::Scalar_type type;
    };

    class Unsupported_return_type_error: public Call_error
    {
    public:
      explicit Unsupported_return_type_error(bytecode::Scalar_type type)
          : Call_error{"unsupported return type"}, type{type}
      {
      }

      bytecode::Scalar_type type;
    };

    Virtual_machine();

    Pointer lookup_constant(bytecode::Constant k) const
    {
      // TODO: check that k is in range
      return Pointer{
        Address_space::constant,
        static_cast<uint64_t>(module->constant_table[k.value])
      };
    }

    template <typename T>
    [[nodiscard]] T get_constant_value(bytecode::Constant k) const
    {
      // TODO: check that k is in range
      auto const offset = module->constant_table[k.value];
      auto value = std::uint64_t{};
      std::memcpy(&value, module->constant_data.data() + offset, sizeof(T));
      if constexpr (std::same_as<T, float>)
      {
        return std::bit_cast<float>(static_cast<std::uint32_t>(value));
      }
      else if constexpr (std::same_as<T, double>)
      {
        return std::bit_cast<double>(value);
      }
      else
      {
        return static_cast<T>(value);
      }
    }

    template <typename T>
    [[nodiscard]] T get_register_value(bytecode::Register reg) const
    {
      auto const value = (*registers)[reg.value];
      if constexpr (std::same_as<T, float>)
      {
        return std::bit_cast<float>(static_cast<std::uint32_t>(value));
      }
      else if constexpr (std::same_as<T, double>)
      {
        return std::bit_cast<double>(value);
      }
      else if constexpr (std::same_as<T, bool>)
      {
        return value != 0;
      }
      else
      {
        return static_cast<T>(value);
      }
    }

    template <typename T>
    void set_register_value(bytecode::Register reg, T value)
    {
      if constexpr (std::same_as<std::decay_t<T>, float>)
      {
        (*registers)[reg.value] = std::bit_cast<std::uint32_t>(value);
      }
      else if constexpr (std::same_as<std::decay_t<T>, double>)
      {
        (*registers)[reg.value] = std::bit_cast<std::uint64_t>(value);
      }
      else if constexpr (std::same_as<T, bool>)
      {
        (*registers)[reg.value] = value ? 1 : 0;
      }
      else
      {
        (*registers)[reg.value] = static_cast<std::uint64_t>(value);
      }
    }

    void load(bytecode::Module const &module);

    void run();

    /// Looks up a function by `name` in the loaded module, pushes `args` on
    /// the stack at their native widths, jumps to the entry, runs until the
    /// callee returns, and decodes the return value from `gpr(1)` according to
    /// the function's return type.
    ///
    /// Precondition: a module has been loaded via `load`. Violating this is a
    /// programming error and aborts via `assert`.
    ///
    /// Throws:
    /// - `Unknown_function_error` — `name` is not in
    ///   `module->function_indices`.
    /// - `Argument_count_error` — `args.size()` differs from the function's
    ///   declared parameter count.
    /// - `Argument_type_error` — an `args[i]` does not hold the
    ///   `Scalar` alternative matching the i-th parameter type.
    /// - `Unsupported_argument_type_error` — the i-th parameter type is not a
    ///   primitive type the VM knows how to marshal.
    /// - `Unsupported_return_type_error` — the function's return type is not a
    ///   primitive type the VM knows how to decode.
    Scalar call(Spelling name, std::span<Scalar const> args);

    bytecode::Module const *module{};
    std::byte const *instruction_pointer;
    std::unique_ptr<std::array<std::uint64_t, 64 * 1024>> registers;
    std::unique_ptr<std::array<std::byte, 16 * 1024 * 1024>> stack;
    std::ptrdiff_t stack_pointer{};

  private:
    void dispatch(bytecode::Opcode opcode);

    void wide_dispatch(bytecode::Opcode opcode);
  };

} // namespace benson

#endif // BENSON_VM_VM_H
