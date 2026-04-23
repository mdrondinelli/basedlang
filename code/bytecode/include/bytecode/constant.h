#ifndef BENSON_BYTECODE_CONSTANT_INDEX_H
#define BENSON_BYTECODE_CONSTANT_INDEX_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>

namespace benson::bytecode
{

  struct Constant
  {
    using Underlying_type = std::uint8_t;
    Underlying_type value;

    constexpr Constant() = default;

    template <std::integral T>
    constexpr explicit Constant(T v)
        : value{
            (assert(std::in_range<Underlying_type>(v)),
             static_cast<Underlying_type>(v))
          }
    {
    }
  };

  struct Wide_constant
  {
    using Underlying_type = std::uint16_t;
    Underlying_type value;

    constexpr Wide_constant() = default;

    template <std::integral T>
    constexpr explicit Wide_constant(T v)
        : value{
            (assert(std::in_range<Underlying_type>(v)),
             static_cast<Underlying_type>(v))
          }
    {
    }

    constexpr Wide_constant(Constant c) : value{c.value}
    {
    }
  };

}

#endif // BENSON_BYTECODE_CONSTANT_INDEX_H
