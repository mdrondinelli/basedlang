#ifndef BASED_FRONTEND_SYMBOL_TABLE_H
#define BASED_FRONTEND_SYMBOL_TABLE_H

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include <hlir/constant_value.h>
#include <hlir/hlir.h>
#include <hlir/type.h>
#include <spelling/spelling.h>

namespace benson
{

  struct Object_binding
  {
    hlir::Type *type;
    bool is_mutable;
    hlir::Register reg;
  };

  struct Symbol
  {
    Spelling name;
    std::variant<Object_binding, hlir::Constant_value> data;
  };

  class Symbol_table
  {
  public:
    Symbol *declare_object(
      Spelling name,
      hlir::Type *type,
      bool is_mutable,
      hlir::Register reg = {}
    );

    Symbol *declare_value(Spelling name, hlir::Constant_value const &value);

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

} // namespace benson

#endif
