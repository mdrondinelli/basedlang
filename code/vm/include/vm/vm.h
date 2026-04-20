#ifndef BENSON_VM_VM_H
#define BENSON_VM_VM_H

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

#include "bytecode/constant.h"
#include "bytecode/module.h"
#include "bytecode/opcode.h"
#include "bytecode/register.h"
#include "pointer.h"

namespace benson
{

  class Virtual_machine
  {
  public:
    Virtual_machine();

    Pointer lookup_constant(bytecode::Wide_constant k) const
    {
      // TODO: check that k is in range
      return Pointer{
        Address_space::constant,
        static_cast<uint64_t>(constant_table[static_cast<std::size_t>(k)])
      };
    }

    template <typename T>
    [[nodiscard]] T get_constant_value(bytecode::Wide_constant k) const
    {
      // TODO: check that k is in range
      auto const offset = constant_table[static_cast<std::size_t>(k)];
      auto value = std::uint64_t{};
      std::memcpy(&value, constant_memory + offset, sizeof(T));
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
      auto const value = registers[static_cast<std::size_t>(reg)];
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
    void set_register_value(bytecode::Register reg, T value)
    {
      if constexpr (std::same_as<T, float>)
      {
        registers[static_cast<std::size_t>(reg)] =
          std::bit_cast<std::uint32_t>(value);
      }
      else if constexpr (std::same_as<T, double>)
      {
        registers[static_cast<std::size_t>(reg)] =
          std::bit_cast<std::uint64_t>(value);
      }
      else
      {
        registers[static_cast<std::size_t>(reg)] =
          static_cast<std::uint64_t>(value);
      }
    }

    void load(bytecode::Module const &module);

    void run();

    std::byte const *instruction_pointer;
    std::byte const *constant_memory;
    std::ptrdiff_t const *constant_table;
    std::array<std::uint64_t, 256> registers;
    std::unique_ptr<std::array<std::byte, 4096>> stack;

  private:
    void dispatch(bytecode::Opcode opcode);

    void wide_dispatch(bytecode::Opcode opcode);
  };

} // namespace benson

#endif // BENSON_VM_VM_H
