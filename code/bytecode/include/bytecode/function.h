#ifndef BENSON_BYTECODE_FUNCTION_H
#define BENSON_BYTECODE_FUNCTION_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>

namespace benson::bytecode
{

  struct Function
  {
    using Underlying_type = std::uint16_t;

    Underlying_type value{};

    constexpr Function() = default;

    template <std::integral T>
    constexpr explicit Function(T v)
        : value{
            (assert(std::in_range<Underlying_type>(v)),
             static_cast<Underlying_type>(v))
          }
    {
    }
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_FUNCTION_H
