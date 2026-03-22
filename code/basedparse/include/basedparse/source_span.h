#ifndef BASEDPARSE_SOURCE_SPAN_H
#define BASEDPARSE_SOURCE_SPAN_H

#include "ast.h"
#include "source_location.h"

namespace basedparse
{

  struct Source_span
  {
    Source_location start;
    Source_location end;
  };

  Source_span source_span(Type_expression const &node);
  Source_span source_span(Identifier_type_expression const &node);
  Source_span source_span(Array_type_expression const &node);
  Source_span source_span(Pointer_type_expression const &node);

  Source_span source_span(Expression const &node);
  Source_span source_span(Int_literal_expression const &node);
  Source_span source_span(Identifier_expression const &node);
  Source_span source_span(Fn_expression const &node);
  Source_span source_span(Paren_expression const &node);
  Source_span source_span(Unary_expression const &node);
  Source_span source_span(Binary_expression const &node);
  Source_span source_span(Call_expression const &node);
  Source_span source_span(Index_expression const &node);
  Source_span source_span(Block_expression const &node);
  Source_span source_span(If_expression const &node);
  Source_span source_span(Constructor_expression const &node);

  Source_span source_span(Statement const &node);
  Source_span source_span(Let_statement const &node);
  Source_span source_span(While_statement const &node);
  Source_span source_span(Return_statement const &node);
  Source_span source_span(Expression_statement const &node);
  Source_span source_span(Function_definition const &node);

} // namespace basedparse

#endif // BASEDPARSE_SOURCE_SPAN_H
