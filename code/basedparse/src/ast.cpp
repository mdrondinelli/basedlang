#include "basedparse/ast.h"

namespace basedparse
{

  Array_type_expression::~Array_type_expression() = default;
  Pointer_type_expression::~Pointer_type_expression() = default;
  Fn_expression::~Fn_expression() = default;
  Paren_expression::~Paren_expression() = default;
  Unary_expression::~Unary_expression() = default;
  Binary_expression::~Binary_expression() = default;
  Call_expression::~Call_expression() = default;
  Index_expression::~Index_expression() = default;
  Block_expression::~Block_expression() = default;
  If_expression::Else_if_part::~Else_if_part() = default;
  If_expression::~If_expression() = default;
  While_statement::~While_statement() = default;
  Constructor_expression::~Constructor_expression() = default;

} // namespace basedparse
