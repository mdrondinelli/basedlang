#ifndef BENSON_VM_SCALAR_H
#define BENSON_VM_SCALAR_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <variant>

#include "bytecode/scalar_type.h"

namespace benson::vm
{

  struct Scalar
  {
  public:
    struct Void
    {
      constexpr Void() = default;
    };

    static Scalar const void_;

    constexpr Scalar(std::int8_t value)
        : _value{value}
    {
    }

    constexpr Scalar(std::int16_t value)
        : _value{value}
    {
    }

    constexpr Scalar(std::int32_t value)
        : _value{value}
    {
    }

    constexpr Scalar(std::int64_t value)
        : _value{value}
    {
    }

    Scalar(float value)
        : _value{value}
    {
    }

    Scalar(double value)
        : _value{value}
    {
    }

    Scalar(bool value)
        : _value{value}
    {
    }

    bytecode::Scalar_type type() const noexcept
    {
      return static_cast<bytecode::Scalar_type>(_value.index());
    }

    std::span<std::byte const> bytes() const noexcept
    {
      return std::visit(
        [](auto const &value) -> std::span<std::byte const>
        {
          return std::as_bytes(std::span{&value, std::size_t{1}});
        },
        _value
      );
    }

    template <typename T>
    [[nodiscard]] T as() const noexcept
    {
      assert(std::holds_alternative<T>(_value));
      return std::get<T>(_value);
    }

  private:
    constexpr Scalar(Void)
        : _value{Void{}}
    {
    }

    std::variant<
      std::int8_t,
      std::int16_t,
      std::int32_t,
      std::int64_t,
      float,
      double,
      bool,
      Void
    >
      _value;
  };

  inline Scalar const Scalar::void_{Scalar::Void{}};
} // namespace benson::vm

#endif // BENSON_VM_SCALAR_H
