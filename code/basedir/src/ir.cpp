#include "basedir/ir.h"

namespace basedir
{

  Expression::Expression(Expression::Variant v, Type *t)
      : value{std::move(v)}, type{t}
  {
  }

  Unary_expression::~Unary_expression() = default;

  Binary_expression::~Binary_expression() = default;

  Assign_expression::~Assign_expression() = default;

  Call_expression::~Call_expression() = default;

  Block_expression::~Block_expression() = default;

  Index_expression::~Index_expression() = default;

  If_expression::~If_expression() = default;

  While_statement::~While_statement() = default;

  Function_body::~Function_body() = default;

} // namespace basedir
