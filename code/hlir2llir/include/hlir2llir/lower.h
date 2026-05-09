#ifndef BENSON_HLIR2LLIR_LOWER_H
#define BENSON_HLIR2LLIR_LOWER_H

#include <hlir/hlir.h>
#include <llir/llir.h>

namespace benson::hlir2llir
{

  llir::Translation_unit lower(hlir::Translation_unit const &tu);

} // namespace benson::hlir2llir

#endif // BENSON_HLIR2LLIR_LOWER_H
