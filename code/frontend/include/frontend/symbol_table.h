#ifndef BASED_FRONTEND_SYMBOL_TABLE_H
#define BASED_FRONTEND_SYMBOL_TABLE_H

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include <ir/constant_value.h>
#include <ir/hlir.h>
#include <ir/type.h>
#include <spelling/spelling.h>

namespace benson::ir
{

  struct Object_binding
  {
    Type *type;
    bool is_mutable;
    Register reg;
  };

  struct Symbol
  {
    Spelling name;
    std::variant<Object_binding, Constant_value> data;
  };

  class Symbol_table
  {
  public:
    Symbol *declare_object(
      Spelling name,
      Type *type,
      bool is_mutable,
      Register reg = {}
    );

    Symbol *declare_value(Spelling name, Constant_value const &value);

    Symbol *lookup(Spelling name) const;

    void push_scope(bool is_barrier = false);

    void pop_scope();

  private:
    struct Scope
    {
      std::unordered_map<Spelling, Symbol *> symbols;
      bool is_barrier;
    };

    std::vector<std::unique_ptr<Symbol>> _symbols;
    std::unordered_map<Spelling, Symbol *> _global_scope;
    std::vector<Scope> _scopes;
  };

} // namespace benson::ir

#endif
