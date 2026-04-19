#ifndef BENSON_VM_VM_H
#define BENSON_VM_VM_H

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "bytecode/register.h"

namespace benson
{

  class Virtual_machine
  {
  public:
    Virtual_machine();

    template <typename T>
    [[nodiscard]] auto get_register_value(bytecode::Register reg) const -> T
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

    void run();

    std::byte const *instruction_pointer;
    std::array<std::uint64_t, 256> registers;
    std::unique_ptr<std::array<std::byte, 4096>> stack;
  };

} // namespace benson

#endif // BENSON_VM_VM_H
