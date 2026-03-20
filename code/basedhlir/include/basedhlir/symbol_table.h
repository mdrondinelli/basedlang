#ifndef BASEDHLIR_SYMBOL_TABLE_H
#define BASEDHLIR_SYMBOL_TABLE_H

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

  struct Symbol
  {
    std::string name;
    std::variant<Value_symbol, Type_symbol> data;
  };

  class Symbol_table
  {
  public:
    void push_scope();

    void pop_scope();

    Symbol *declare_value(std::string name, Type *type, bool is_mutable);

    Symbol *declare_type(std::string name, Type *type);

    Symbol *lookup(std::string_view name) const;

  private:
    std::vector<std::unique_ptr<Symbol>> _symbols;
    std::vector<std::unordered_map<std::string_view, Symbol *>> _scopes;
  };

} // namespace basedhlir

#endif
