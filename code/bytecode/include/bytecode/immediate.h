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
    using Underlying_type = std::int16_t;

    Underlying_type value{};

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

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_IMMEDIATE_H
