#ifndef BENSON_VM_POINTER_H
#define BENSON_VM_POINTER_H

#include <cstdint>

namespace benson::vm
{
  enum class Address_space : uint64_t
  {
    constant = 0xC000000000000000u,
    stack = 0x4000000000000000u,
    heap = 0x8000000000000000u,
  };

  struct Pointer_decode_result
  {
    Address_space space;
    std::uint64_t offset;
  };

  struct Pointer
  {
    Pointer(Address_space space, std::uint64_t offset)
        : _value{static_cast<std::uint64_t>(space) | offset}
    {
    }

    explicit Pointer(std::uint64_t value)
        : _value{value}
    {
    }

    explicit operator std::uint64_t() const
    {
      return _value;
    }

    Pointer_decode_result decode()
    {
      return {
        .space = static_cast<Address_space>(_value & 0xC000000000000000u),
        .offset = static_cast<std::uint64_t>(_value & 0x3FFFFFFFFFFFFFFFu),
      };
    }

  private:
    std::uint64_t _value;
  };
} // namespace benson::vm

#endif // BENSON_VM_ADDRESS_SPACE_H
