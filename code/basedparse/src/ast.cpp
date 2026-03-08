#include "basedparse/expression.h"
#include "basedparse/statement.h"
#include "basedparse/type_expression.h"

namespace basedparse
{

  // Defined out-of-line because the default destructor needs Statement to be
  // complete (for the vector<unique_ptr<Statement>> member).
  Fn_expression::~Fn_expression() = default;

  // Defined out-of-line because the default destructor needs Expression to be
  // complete (for the unique_ptr<Expression> size member).
  Array_type_expression::~Array_type_expression() = default;

} // namespace basedparse
