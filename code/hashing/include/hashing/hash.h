#ifndef BENSON_HASHING_HASH_H
#define BENSON_HASHING_HASH_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace benson
{

  struct Hash_state
  {
    std::uint64_t value{14695981039346656037ull};

    [[nodiscard]] auto mix(std::span<std::byte const> bytes) const noexcept
      -> Hash_state
    {
      auto result = *this;
      for (auto const byte : bytes)
      {
        result.value ^= static_cast<std::uint64_t>(byte);
        result.value *= 1099511628211ull;
      }
      return result;
    }

    template <typename T>
    [[nodiscard]] auto mix(T const &value) const noexcept -> Hash_state
    {
      static_assert(std::is_integral_v<T>);
      return mix(std::as_bytes(std::span{&value, 1}));
    }

    template <typename T>
    [[nodiscard]] auto mix(T *value) const noexcept -> Hash_state
    {
      return mix(reinterpret_cast<std::uintptr_t>(value));
    }

    template <typename First, typename... Rest>
    [[nodiscard]] auto
    mix(First const &first, Rest const &...rest) const noexcept -> Hash_state
    {
      auto state = mix(first);
      ((state = state.mix(rest)), ...);
      return state;
    }
  };

  [[nodiscard]] inline auto
  hash_bytes(std::span<std::byte const> bytes) noexcept -> Hash_state
  {
    return Hash_state{}.mix(bytes);
  }

  template <typename... Values>
  [[nodiscard]] auto hash_values(Values const &...values) noexcept
    -> std::uint64_t
  {
    return Hash_state{}.mix(values...).value;
  }

} // namespace benson

#endif // BENSON_HASHING_HASH_H
