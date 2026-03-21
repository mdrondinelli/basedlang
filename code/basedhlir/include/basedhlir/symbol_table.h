#ifndef BASEDHLIR_SYMBOL_TABLE_H
#define BASEDHLIR_SYMBOL_TABLE_H

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "type.h"

namespace basedhlir
{

  struct Value_symbol
  {
    Type *type;
    bool is_mutable;
  };

  struct Type_symbol
  {
    Type *type;
  };

  struct Function_symbol
  {
    std::uint64_t function_handle;
  };

  struct Symbol
  {
    std::string name;
    std::variant<Value_symbol, Type_symbol, Function_symbol> data;
  };

  class Symbol_table
  {
  public:
    Symbol *declare_value(std::string name, Type *type, bool is_mutable);

    Symbol *declare_type(std::string name, Type *type);

    Symbol *declare_function(std::string name, std::uint64_t function_handle);

    Symbol *lookup(std::string_view name) const;

    void push_scope(bool is_barrier = false);

    void pop_scope();

  private:
    struct Scope
    {
      std::unordered_map<std::string_view, Symbol *> symbols;
      bool is_barrier;
    };

    std::vector<std::unique_ptr<Symbol>> _symbols;
    std::unordered_map<std::string_view, Symbol *> _global_scope;
    std::vector<Scope> _scopes;
  };

} // namespace basedhlir

#endif
