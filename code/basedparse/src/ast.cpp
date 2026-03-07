#include "basedparse/expression.h"
#include "basedparse/statement.h"

namespace basedparse
{

  // Defined out-of-line because the default destructor needs Statement to be
  // complete (for the vector<unique_ptr<Statement>> member).
  Fn_expression::~Fn_expression() = default;

} // namespace basedparse
