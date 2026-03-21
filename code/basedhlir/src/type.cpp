#include <bit>
#include <cstdint>

#include "basedhlir/type.h"

namespace basedhlir
{

  auto Type_pool::Function_type_hash::operator()(Function_type const &ft) const noexcept -> std::size_t
  {
    auto seed = std::bit_cast<std::uintptr_t>(ft.return_type);
    for (auto const param : ft.parameter_types)
    {
      seed ^= std::bit_cast<std::uintptr_t>(param) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }

  Type *Type_pool::i32_type()
  {
    if (_i32_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(I32_type{}));
      _i32_type = _types.back().get();
    }
    return _i32_type;
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

  Type *Type_pool::pointer_type(Type *pointee, bool is_mutable)
  {
    auto &cached = is_mutable ? pointee->_mut_pointer_type : pointee->_pointer_type;
    if (cached == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Pointer_type{.pointee = pointee, .is_mutable = is_mutable}));
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
    _types.push_back(std::make_unique<Type>(Sized_array_type{.element = element, .size = size}));
    auto const result = _types.back().get();
    element->_sized_array_types.emplace(size, result);
    return result;
  }

  Type *Type_pool::unsized_array_type(Type *element)
  {
    if (element->_unsized_array_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Unsized_array_type{.element = element}));
      element->_unsized_array_type = _types.back().get();
    }
    return element->_unsized_array_type;
  }

  Type *Type_pool::function_type(std::vector<Type *> parameter_types, Type *return_type)
  {
    auto ft = Function_type{.parameter_types = std::move(parameter_types), .return_type = return_type};
    auto const it = _function_types.find(ft);
    if (it != _function_types.end())
    {
      return it->second;
    }
    auto const result = _types.emplace_back(std::make_unique<Type>(ft)).get();
    _function_types.emplace(std::move(ft), result);
    return result;
  }

} // namespace basedhlir
