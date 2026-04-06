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

  struct Int8_type
  {
  };

  struct Int16_type
  {
  };

  struct Int32_type
  {
  };

  struct Int64_type
  {
  };

  struct Bool_type
  {
  };

  struct Void_type
  {
  };

  struct Type_type
  {
  };

  struct Pointer_type
  {
    Type *pointee;
    bool is_mutable;
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

  struct Function_type
  {
    std::vector<Type *> parameter_types;
    Type *return_type;

    auto operator==(Function_type const &other) const -> bool = default;
  };

  struct Type
  {
    std::variant<
      Int8_type,
      Int16_type,
      Int32_type,
      Int64_type,
      Bool_type,
      Void_type,
      Type_type,
      Pointer_type,
      Sized_array_type,
      Unsized_array_type,
      Function_type
    >
      data;

    Type *_pointer_type{};
    Type *_mut_pointer_type{};
    Type *_unsized_array_type{};
    std::unordered_map<std::int64_t, Type *> _sized_array_types;
  };

  bool is_object_type(Type const *type);

  class Type_pool
  {
  public:
    Type *int8_type();

    Type *int16_type();

    Type *int32_type();

    Type *int64_type();

    Type *bool_type();

    Type *void_type();

    Type *type_type();

    Type *pointer_type(Type *pointee, bool is_mutable);

    Type *sized_array_type(Type *element, std::int64_t size);

    Type *unsized_array_type(Type *element);

    Type *function_type(std::vector<Type *> parameter_types, Type *return_type);

  private:
    std::vector<std::unique_ptr<Type>> _types;
    Type *_int8_type{};
    Type *_int16_type{};
    Type *_int32_type{};
    Type *_int64_type{};
    Type *_bool_type{};
    Type *_void_type{};
    Type *_type_type{};

    struct Function_type_hash
    {
      auto operator()(Function_type const &ft) const noexcept -> std::size_t;
    };

    std::unordered_map<Function_type, Type *, Function_type_hash>
      _function_types;
  };

} // namespace basedhlir

#endif
