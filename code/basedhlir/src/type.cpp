#include "basedhlir/type.h"

namespace basedhlir
{

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

  Type *Type_pool::pointer_type(Type *pointee)
  {
    if (pointee->_pointer_type == nullptr)
    {
      _types.push_back(std::make_unique<Type>(Pointer_type{.pointee = pointee}));
      pointee->_pointer_type = _types.back().get();
    }
    return pointee->_pointer_type;
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

} // namespace basedhlir
