#ifndef BENSON_BYTECODE_REGISTER_H
#define BENSON_BYTECODE_REGISTER_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <utility>

namespace benson::bytecode
{

  struct Register
  {
    using Underlying_type = std::uint16_t;

    Underlying_type value{};

    constexpr Register() = default;

    template <std::integral T>
    constexpr explicit Register(T v)
        : value{
            (assert(std::in_range<Underlying_type>(v)),
             static_cast<Underlying_type>(v))
          }
    {
    }
  };

  inline constexpr auto sp = Register{0};

  constexpr auto gpr(int n) -> Register
  {
    assert(n >= 1);
    assert(n <= std::numeric_limits<Register::Underlying_type>::max());
    return Register{n};
  }

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_REGISTER_H
