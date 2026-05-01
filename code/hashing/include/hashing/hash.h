#ifndef BENSON_HASHING_HASH_H
#define BENSON_HASHING_HASH_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

namespace benson
{

  struct Hash_state
  {
    std::uint64_t value{14695981039346656037ull};

    auto mix(std::span<std::byte const> bytes) noexcept -> Hash_state &
    {
      for (auto const byte : bytes)
      {
        value ^= static_cast<std::uint64_t>(byte);
        value *= 1099511628211ull;
      }
      return *this;
    }

    template <typename T>
    auto mix(T const &value) noexcept -> Hash_state &
    {
      static_assert(std::is_integral_v<T>);
      return mix(std::as_bytes(std::span{&value, 1}));
    }

    template <typename T>
    auto mix(T *value) noexcept -> Hash_state &
    {
      return mix(reinterpret_cast<std::uintptr_t>(value));
    }

    template <typename First, typename... Rest>
    auto mix(First const &first, Rest const &...rest) noexcept -> Hash_state &
    {
      mix(first);
      (mix(rest), ...);
      return *this;
    }
  };

  [[nodiscard]] inline auto
  hash_bytes(std::span<std::byte const> bytes) noexcept -> std::uint64_t
  {
    auto state = Hash_state{};
    state.mix(bytes);
    return state.value;
  }

  [[nodiscard]] inline auto hash_text(std::string_view text) noexcept
    -> std::uint64_t
  {
    return hash_bytes(std::as_bytes(std::span{text.data(), text.size()}));
  }

  template <typename... Values>
  [[nodiscard]] auto hash_values(Values const &...values) noexcept
    -> std::uint64_t
  {
    auto state = Hash_state{};
    state.mix(values...);
    return state.value;
  }

} // namespace benson

#endif // BENSON_HASHING_HASH_H
