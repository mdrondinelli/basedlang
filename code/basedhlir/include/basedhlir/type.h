#ifndef BASEDHLIR_TYPE_H
#define BASEDHLIR_TYPE_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace basedhlir
{

  struct Type;

  struct I32_type
  {
  };

  struct Bool_type
  {
  };

  struct Void_type
  {
  };

  struct Pointer_type
  {
    Type *pointee;
  };

  struct Sized_array_type
  {
    Type *element;
    std::int64_t size;
  };

  struct Unsized_array_type
  {
    Type *element;
  };

  struct Type
  {
    std::variant<I32_type, Bool_type, Void_type, Pointer_type, Sized_array_type, Unsized_array_type> data;

    Type *_pointer_type{};
    Type *_unsized_array_type{};
    std::unordered_map<std::int64_t, Type *> _sized_array_types;
  };

  class Type_pool
  {
  public:
    Type *i32_type();

    Type *bool_type();

    Type *void_type();

    Type *pointer_type(Type *pointee);

    Type *sized_array_type(Type *element, std::int64_t size);

    Type *unsized_array_type(Type *element);

  private:
    std::vector<std::unique_ptr<Type>> _types;
    Type *_i32_type{};
    Type *_bool_type{};
    Type *_void_type{};
  };

} // namespace basedhlir

#endif
