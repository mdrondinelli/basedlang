#ifndef BENSON_BYTECODE_IMMEDIATE_H
#define BENSON_BYTECODE_IMMEDIATE_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>

namespace benson::bytecode
{

  struct Immediate
  {
    using Underlying_type = std::int8_t;
    Underlying_type value = 0;

    constexpr Immediate() = default;

    template <std::integral T>
    constexpr explicit Immediate(T v)
        : value{
            (assert(std::in_range<Underlying_type>(v)),
             static_cast<Underlying_type>(v))
          }
    {
    }
  };

  struct Wide_immediate
  {
    using Underlying_type = std::int16_t;
    Underlying_type value = 0;

    constexpr Wide_immediate() = default;

    template <std::integral T>
    constexpr explicit Wide_immediate(T v)
        : value{
            (assert(std::in_range<Underlying_type>(v)),
             static_cast<Underlying_type>(v))
          }
    {
    }

    constexpr Wide_immediate(Immediate i) : value{i.value}
    {
    }
  };

}

#endif // BENSON_BYTECODE_IMMEDIATE_H
