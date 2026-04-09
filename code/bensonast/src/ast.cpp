#include "bensonast/ast.h"

namespace bensonast
{

  Fn_expression::~Fn_expression() = default;
  Paren_expression::~Paren_expression() = default;
  Prefix_expression::~Prefix_expression() = default;
  Postfix_expression::~Postfix_expression() = default;
  Binary_expression::~Binary_expression() = default;
  Call_expression::~Call_expression() = default;
  Index_expression::~Index_expression() = default;
  Prefix_bracket_expression::~Prefix_bracket_expression() = default;
  Block_expression::~Block_expression() = default;
  If_expression::Else_if_part::~Else_if_part() = default;
  If_expression::~If_expression() = default;
  While_statement::~While_statement() = default;

} // namespace bensonast
