#include <string>
#include <utility>

#include "basedir/type.h"

namespace basedir
{

  Type *Type_pool::get_named(std::string_view name)
  {
    auto const found = _named_types.find(name);
    if (found != _named_types.end())
    {
      return found->second;
    }
    if (name == "i32")
    {
      auto const type = alloc(Type{I32_type{}});
      _named_types.emplace(std::string{name}, type);
      return type;
    }
    if (name == "void")
    {
      auto const type = alloc(Type{Void_type{}});
      _named_types.emplace(std::string{name}, type);
      return type;
    }
    return nullptr;
  }

  Type *Type_pool::get_pointer(Type *pointee)
  {
    if (pointee->pointer_type)
    {
      return pointee->pointer_type;
    }
    auto const type = alloc(Type{Pointer_type{pointee}});
    pointee->pointer_type = type;
    return type;
  }

  Type *Type_pool::get_reference(Type *referent)
  {
    if (referent->reference_type)
    {
      return referent->reference_type;
    }
    auto const type = alloc(Type{Reference_type{referent}});
    referent->reference_type = type;
    return type;
  }

  Type *Type_pool::get_function(
    std::span<Type *const> parameter_types,
    Type *return_type
  )
  {
    auto const view = Function_key_view{parameter_types, return_type};
    auto const found = _function_types.find(view);
    if (found != _function_types.end())
    {
      return found->second;
    }
    auto const type = alloc(Type{Function_type{
      std::vector<Type *>{parameter_types.begin(), parameter_types.end()},
      return_type,
    }});
    _function_types.emplace(
      Function_key{
        std::vector<Type *>{parameter_types.begin(), parameter_types.end()},
        return_type,
      },
      type
    );
    return type;
  }

  Type *Type_pool::get_array(Type *element, std::optional<std::size_t> size)
  {
    auto const found = element->array_types.find(size);
    if (found != element->array_types.end())
    {
      return found->second;
    }
    auto const type = alloc(Type{Array_type{element, size}});
    element->array_types.emplace(size, type);
    return type;
  }

  Type *Type_pool::alloc(Type type)
  {
    _types.push_back(std::make_unique<Type>(std::move(type)));
    return _types.back().get();
  }

} // namespace basedir
