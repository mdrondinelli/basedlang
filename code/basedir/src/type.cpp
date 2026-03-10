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

  Type *Type_pool::alloc(Type type)
  {
    _types.push_back(std::make_unique<Type>(std::move(type)));
    return _types.back().get();
  }

} // namespace basedir
