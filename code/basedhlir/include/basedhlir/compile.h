#ifndef BASEDHLIR_COMPILE_H
#define BASEDHLIR_COMPILE_H

#include <basedparse/ast.h>

#include "hlir.h"
#include "type.h"

namespace basedhlir
{

  Translation_unit compile(basedparse::Translation_unit const &ast, Type_pool *type_pool);

} // namespace basedhlir

#endif
