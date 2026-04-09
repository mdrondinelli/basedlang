#ifndef BASEDHLIR_PRIMITIVE_TYPE_TRAITS_H
#define BASEDHLIR_PRIMITIVE_TYPE_TRAITS_H

#include <cstdint>
#include <string_view>

#include "bensonhlir/type.h"

namespace bensonhlir
{

  template <typename T>
  struct Primitive_type_traits;

  template <>
  struct Primitive_type_traits<std::int8_t>
  {
    static constexpr std::string_view name = "Int8";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int8_type();
    }
  };

  template <>
  struct Primitive_type_traits<std::int16_t>
  {
    static constexpr std::string_view name = "Int16";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int16_type();
    }
  };

  template <>
  struct Primitive_type_traits<std::int32_t>
  {
    static constexpr std::string_view name = "Int32";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int32_type();
    }
  };

  template <>
  struct Primitive_type_traits<std::int64_t>
  {
    static constexpr std::string_view name = "Int64";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int64_type();
    }
  };

  template <>
  struct Primitive_type_traits<float>
  {
    static constexpr std::string_view name = "Float32";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->float32_type();
    }
  };

  template <>
  struct Primitive_type_traits<double>
  {
    static constexpr std::string_view name = "Float64";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->float64_type();
    }
  };

  template <>
  struct Primitive_type_traits<bool>
  {
    static constexpr std::string_view name = "Bool";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->bool_type();
    }
  };

} // namespace bensonhlir

#endif
