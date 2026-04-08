#ifndef BASEDHLIR_SYMBOL_TABLE_H
#define BASEDHLIR_SYMBOL_TABLE_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "constant_value.h"
#include "hlir.h"
#include "type.h"

namespace kalehlir
{

  struct Object_binding
  {
    Type *type;
    bool is_mutable;
    Register reg;
  };

  struct Symbol
  {
    std::string name;
    std::variant<Object_binding, Constant_value> data;
  };

  class Symbol_table
  {
  public:
    Symbol *declare_object(
      std::string name,
      Type *type,
      bool is_mutable,
      Register reg = {}
    );

    Symbol *declare_value(std::string name, Constant_value const &value);

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

} // namespace kalehlir

#endif
