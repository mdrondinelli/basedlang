#include <utility>

#include "ir/symbol_table.h"

namespace benson::ir
{

  Symbol *Symbol_table::declare_object(
    std::string name,
    Type *type,
    bool is_mutable,
    Register reg
  )
  {
    auto const sym =
      _symbols
        .emplace_back(
          std::make_unique<Symbol>(
            std::move(name),
            Object_binding{.type = type, .is_mutable = is_mutable, .reg = reg}
          )
        )
        .get();
    if (_scopes.empty())
    {
      _global_scope.insert_or_assign(sym->name, sym);
    }
    else
    {
      _scopes.back().symbols.insert_or_assign(sym->name, sym);
    }
    return sym;
  }

  Symbol *
  Symbol_table::declare_value(std::string name, Constant_value const &value)
  {
    auto const sym =
      _symbols.emplace_back(std::make_unique<Symbol>(std::move(name), value))
        .get();
    if (_scopes.empty())
    {
      _global_scope.insert_or_assign(sym->name, sym);
    }
    else
    {
      _scopes.back().symbols.insert_or_assign(sym->name, sym);
    }
    return sym;
  }

  Symbol *Symbol_table::lookup(std::string_view name) const
  {
    for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it)
    {
      auto const found = it->symbols.find(name);
      if (found != it->symbols.end())
      {
        return found->second;
      }
      if (it->is_barrier)
      {
        break;
      }
    }
    auto const it = _global_scope.find(name);
    if (it != _global_scope.end())
    {
      return it->second;
    }
    return nullptr;
  }

  void Symbol_table::push_scope(bool is_barrier)
  {
    _scopes.push_back({.symbols = {}, .is_barrier = is_barrier});
  }

  void Symbol_table::pop_scope()
  {
    _scopes.pop_back();
  }

} // namespace benson::ir
