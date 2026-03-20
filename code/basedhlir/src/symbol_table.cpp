#include "basedhlir/symbol_table.h"

#include <utility>

namespace basedhlir
{

  void Symbol_table::push_scope()
  {
    _scopes.emplace_back();
  }

  void Symbol_table::pop_scope()
  {
    _scopes.pop_back();
  }

  Symbol *Symbol_table::declare_value(std::string name, Type *type, bool is_mutable)
  {
    auto const sym = _symbols.emplace_back(
      std::make_unique<Symbol>(std::move(name), Value_symbol{.type = type, .is_mutable = is_mutable})
    ).get();
    _scopes.back().insert_or_assign(sym->name, sym);
    return sym;
  }

  Symbol *Symbol_table::declare_type(std::string name, Type *type)
  {
    auto const sym = _symbols.emplace_back(
      std::make_unique<Symbol>(std::move(name), Type_symbol{.type = type})
    ).get();
    _scopes.back().insert_or_assign(sym->name, sym);
    return sym;
  }

  Symbol *Symbol_table::lookup(std::string_view name) const
  {
    for (auto i = _scopes.size(); i > 0; --i)
    {
      auto const it = _scopes[i - 1].find(name);
      if (it != _scopes[i - 1].end())
      {
        return it->second;
      }
    }
    return nullptr;
  }

} // namespace basedhlir
