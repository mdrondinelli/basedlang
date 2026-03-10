#ifndef BASEDIR_TYPE_H
#define BASEDIR_TYPE_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace basedir
{

  struct Type;

  struct Void_type
  {
  };

  struct I32_type
  {
  };

  struct Array_type
  {
    Type *element;
    std::optional<std::size_t> size;
  };

  struct Function_type
  {
    std::vector<Type *> parameter_types;
    Type *return_type;
  };

  struct Pointer_type
  {
    Type *pointee;
    bool is_mutable;
  };

  struct Reference_type
  {
    Type *referent;
    bool is_mutable;
  };

  struct Type
  {
    using Variant = std::variant<
      Void_type,
      I32_type,
      Array_type,
      Function_type,
      Pointer_type,
      Reference_type
    >;

    explicit Type(Variant v)
        : value{std::move(v)}
    {
    }

    Variant value;
    Type *pointer_type = nullptr;
    Type *mut_pointer_type = nullptr;
    Type *reference_type = nullptr;
    Type *mut_reference_type = nullptr;
    std::unordered_map<std::optional<std::size_t>, Type *> array_types;
  };

  // Interning pool for types. Two types are equal iff their pointers are equal.
  // Not thread-safe — all methods require external synchronization.
  class Type_pool
  {
  public:
    // Returns the named type, or nullptr if unknown. Lazily creates built-in
    // types (e.g. i32) on first access.
    Type *get_named(std::string_view name);

    // Returns the (immutable) pointer-to-pointee type, lazily creating it if
    // needed.
    Type *get_pointer(Type *pointee);

    // Returns the mutable pointer-to-pointee type, lazily creating it if
    // needed.
    Type *get_mut_pointer(Type *pointee);

    // Returns the (immutable) reference-to-referent type.
    Type *get_reference(Type *referent);

    // Returns the mutable reference-to-referent type.
    Type *get_mut_reference(Type *referent);

    // Returns the array-of-element type, lazily creating it if needed.
    Type *get_array(Type *element, std::optional<std::size_t> size);

    // Returns a function type with the given parameter and return types.
    Type *
    get_function(std::span<Type * const> parameter_types, Type *return_type);

  private:
    struct String_hash
    {
      using is_transparent = void;

      std::size_t operator()(std::string_view sv) const
      {
        return std::hash<std::string_view>{}(sv);
      }
    };

    struct Function_key
    {
      std::vector<Type *> parameter_types;
      Type *return_type;
    };

    struct Function_key_view
    {
      std::span<Type * const> parameter_types;
      Type *return_type;
    };

    struct Function_key_hash
    {
      using is_transparent = void;

      static std::size_t
      hash(std::span<Type * const> parameter_types, Type *return_type)
      {
        auto h = std::hash<Type *>{}(return_type);
        for (auto const p : parameter_types)
        {
          h ^= std::hash<Type *>{}(p) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
      }

      std::size_t operator()(Function_key const &key) const
      {
        return hash(key.parameter_types, key.return_type);
      }

      std::size_t operator()(Function_key_view const &key) const
      {
        return hash(key.parameter_types, key.return_type);
      }
    };

    struct Function_key_equal
    {
      using is_transparent = void;

      static bool equal(
        std::span<Type * const> a_params,
        Type *a_ret,
        std::span<Type * const> b_params,
        Type *b_ret
      )
      {
        return a_ret == b_ret && std::equal(
                                   a_params.begin(),
                                   a_params.end(),
                                   b_params.begin(),
                                   b_params.end()
                                 );
      }

      bool operator()(Function_key const &a, Function_key const &b) const
      {
        return equal(
          a.parameter_types,
          a.return_type,
          b.parameter_types,
          b.return_type
        );
      }

      bool operator()(Function_key const &a, Function_key_view const &b) const
      {
        return equal(
          a.parameter_types,
          a.return_type,
          b.parameter_types,
          b.return_type
        );
      }

      bool operator()(Function_key_view const &a, Function_key const &b) const
      {
        return equal(
          a.parameter_types,
          a.return_type,
          b.parameter_types,
          b.return_type
        );
      }
    };

    std::vector<std::unique_ptr<Type>> _types;
    std::unordered_map<std::string, Type *, String_hash, std::equal_to<>>
      _named_types;
    std::
      unordered_map<Function_key, Type *, Function_key_hash, Function_key_equal>
        _function_types;

    Type *alloc(Type type);
  };

} // namespace basedir

#endif // BASEDIR_TYPE_H
