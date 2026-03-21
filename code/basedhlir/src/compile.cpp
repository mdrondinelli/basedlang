#include <cassert>

#include "basedhlir/compile.h"
#include "basedhlir/symbol_table.h"

namespace basedhlir
{

  namespace
  {

    class Compilation_context
    {
    public:
      explicit Compilation_context(Type_pool *type_pool)
        : _type_pool{type_pool}
      {
        assert(_type_pool != nullptr);
      }

      Translation_unit compile(basedparse::Translation_unit const &)
      {
        return Translation_unit{};
      }

    private:
      [[maybe_unused]] Type_pool *_type_pool;
      [[maybe_unused]] Symbol_table _symbol_table;
    };

  } // namespace

  Translation_unit compile(basedparse::Translation_unit const &ast, Type_pool *type_pool)
  {
    auto ctx = Compilation_context{type_pool};
    return ctx.compile(ast);
  }

} // namespace basedhlir
