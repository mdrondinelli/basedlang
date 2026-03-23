#ifndef BASEDHLIR_COMPILE_H
#define BASEDHLIR_COMPILE_H

#include <stdexcept>
#include <string>
#include <vector>

#include <basedparse/ast.h>

#include "hlir.h"
#include "source_location.h"
#include "type.h"

namespace basedhlir
{

  struct Diagnostic
  {
    std::string message;
    Source_location location;
  };

  class Compilation_failure : public std::exception
  {
  public:
    explicit Compilation_failure(std::vector<Diagnostic> diagnostics)
      : _diagnostics{std::move(diagnostics)}
    {
    }

    std::vector<Diagnostic> const &diagnostics() const
    {
      return _diagnostics;
    }

    char const *what() const noexcept override
    {
      return "compilation failed";
    }

  private:
    std::vector<Diagnostic> _diagnostics;
  };

  Translation_unit compile(basedparse::Translation_unit const &ast, Type_pool *type_pool);

} // namespace basedhlir

#endif
