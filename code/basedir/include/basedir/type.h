#ifndef BASEDIR_TYPE_H
#define BASEDIR_TYPE_H

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace basedir
{

  struct Type;

  struct I32_type
  {
  };

  struct Pointer_type
  {
    Type *pointee;
  };

  struct Reference_type
  {
    Type *referent;
  };

  struct Type
  {
    std::variant<I32_type, Pointer_type, Reference_type> value;
    Type *pointer_type = nullptr;
    Type *reference_type = nullptr;
  };

  // Interning pool for types. Two types are equal iff their pointers are equal.
  // Not thread-safe — all methods require external synchronization.
  class Type_pool
  {
  public:
    // Returns the named type, or nullptr if unknown. Lazily creates built-in
    // types (e.g. i32) on first access.
    Type *get_named(std::string_view name);

    // Returns the pointer-to-pointee type, lazily creating it if needed.
    Type *get_pointer(Type *pointee);

    // Returns the reference-to-referent type, lazily creating it if needed.
    Type *get_reference(Type *referent);

  private:
    struct String_hash
    {
      using is_transparent = void;

      std::size_t operator()(std::string_view sv) const
      {
        return std::hash<std::string_view>{}(sv);
      }
    };

    std::vector<std::unique_ptr<Type>> _types;
    std::unordered_map<std::string, Type *, String_hash, std::equal_to<>>
      _named_types;

    Type *alloc(Type type);
  };

} // namespace basedir

#endif // BASEDIR_TYPE_H
