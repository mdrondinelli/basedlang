#include <bit>
#include <cstdint>

#include "kalehlir/type.h"

namespace kalehlir
{

  bool is_object_type(Type const *type)
  {
    return !std::holds_alternative<Type_type>(type->data) &&
           !std::holds_alternative<Function_type>(type->data);
  }

auto Type_pool::Function_type_hash::operator()(
    Function_type const &ft
  ) const noexcept -> std::size_t
  {
    auto seed = std::bit_cast<std::uintptr_t>(ft.return_type);
    for (auto const param : ft.parameter_types)
    {
      seed ^= std::bit_cast<std::uintptr_t>(param) + 0x9e3779b9 + (seed << 6) +
              (seed >> 2);
    }
    return seed;
  }

  Type *Type_pool::int8_type()
  {
    if (_int8_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Int8_type{}));
      _int8_type = _types.back().get();
    }
    return _int8_type;
  }

  Type *Type_pool::int16_type()
  {
    if (_int16_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Int16_type{}));
      _int16_type = _types.back().get();
    }
    return _int16_type;
  }

  Type *Type_pool::int32_type()
  {
    if (_int32_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Int32_type{}));
      _int32_type = _types.back().get();
    }
    return _int32_type;
  }

  Type *Type_pool::int64_type()
  {
    if (_int64_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Int64_type{}));
      _int64_type = _types.back().get();
    }
    return _int64_type;
  }

  Type *Type_pool::float32_type()
  {
    if (_float32_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Float32_type{}));
      _float32_type = _types.back().get();
    }
    return _float32_type;
  }

  Type *Type_pool::float64_type()
  {
    if (_float64_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Float64_type{}));
      _float64_type = _types.back().get();
    }
    return _float64_type;
  }

  Type *Type_pool::bool_type()
  {
    if (_bool_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Bool_type{}));
      _bool_type = _types.back().get();
    }
    return _bool_type;
  }

  Type *Type_pool::void_type()
  {
    if (_void_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Void_type{}));
      _void_type = _types.back().get();
    }
    return _void_type;
  }

  Type *Type_pool::type_type()
  {
    if (_type_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Type_type{}));
      _type_type = _types.back().get();
    }
    return _type_type;
  }

  Type *Type_pool::pointer_type(Type *pointee, bool is_mutable)
  {
    auto &cached =
      is_mutable ? pointee->_mut_pointer_type : pointee->_pointer_type;
    if (cached == nullptr)
    {
      _types.push_back(
        std::make_unique<Type>(
          Pointer_type{.pointee = pointee, .is_mutable = is_mutable}
        )
      );
      cached = _types.back().get();
    }
    return cached;
  }

  Type *Type_pool::sized_array_type(Type *element, std::int64_t size)
  {
    auto const it = element->_sized_array_types.find(size);
    if (it != element->_sized_array_types.end())
    {
      return it->second;
    }
    _types.push_back(
      std::make_unique<Type>(Sized_array_type{.element = element, .size = size})
    );
    auto const result = _types.back().get();
    element->_sized_array_types.emplace(size, result);
    return result;
  }

  Type *Type_pool::unsized_array_type(Type *element)
  {
    if (element->_unsized_array_type == nullptr)
    {
      _types.push_back(
        std::make_unique<Type>(Unsized_array_type{.element = element})
      );
      element->_unsized_array_type = _types.back().get();
    }
    return element->_unsized_array_type;
  }

  Type *Type_pool::function_type(
    std::vector<Type *> parameter_types,
    Type *return_type
  )
  {
    auto ft = Function_type{
      .parameter_types = std::move(parameter_types),
      .return_type = return_type
    };
    auto const it = _function_types.find(ft);
    if (it != _function_types.end())
    {
      return it->second;
    }
    auto const result = _types.emplace_back(std::make_unique<Type>(ft)).get();
    _function_types.emplace(std::move(ft), result);
    return result;
  }

} // namespace kalehlir
