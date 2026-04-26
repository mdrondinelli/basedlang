#ifndef BASEDAST_SOURCE_SPAN_H
#define BASEDAST_SOURCE_SPAN_H

#include <source/source_span.h>

#include "ast.h"

namespace benson::ast
{

  /// @name AST span_of overloads
  /// Each overload returns the source span of the corresponding AST node,
  /// from the first character of its leftmost token to the last character of
  /// its rightmost token.
  /// @{

  Source_span span_of(Expression const &node);
  Source_span span_of(Int_literal_expression const &node);
  Source_span span_of(Float_literal_expression const &node);
  Source_span span_of(Identifier_expression const &node);
  Source_span span_of(Recurse_expression const &node);
  Source_span span_of(Fn_expression const &node);
  Source_span span_of(Paren_expression const &node);
  Source_span span_of(Prefix_expression const &node);

  Source_span span_of(Postfix_expression const &node);
  Source_span span_of(Binary_expression const &node);
  Source_span span_of(Call_expression const &node);
  Source_span span_of(Index_expression const &node);
  Source_span span_of(Prefix_bracket_expression const &node);
  Source_span span_of(Block_expression const &node);
  Source_span span_of(If_expression const &node);

  Source_span span_of(Statement const &node);
  Source_span span_of(Let_statement const &node);
  Source_span span_of(While_statement const &node);
  Source_span span_of(Return_statement const &node);
  Source_span span_of(Expression_statement const &node);

  /// @}

} // namespace benson::ast

#endif // BASEDAST_SOURCE_SPAN_H
