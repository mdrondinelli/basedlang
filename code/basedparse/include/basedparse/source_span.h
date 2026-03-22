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

  Source_span span_of(basedlex::Lexeme const &lexeme);

  Source_span hull(Source_span begin, Source_span end);

  Source_span span_of(Type_expression const &node);
  Source_span span_of(Identifier_type_expression const &node);
  Source_span span_of(Array_type_expression const &node);
  Source_span span_of(Pointer_type_expression const &node);

  Source_span span_of(Expression const &node);
  Source_span span_of(Int_literal_expression const &node);
  Source_span span_of(Identifier_expression const &node);
  Source_span span_of(Fn_expression const &node);
  Source_span span_of(Paren_expression const &node);
  Source_span span_of(Unary_expression const &node);
  Source_span span_of(Binary_expression const &node);
  Source_span span_of(Call_expression const &node);
  Source_span span_of(Index_expression const &node);
  Source_span span_of(Block_expression const &node);
  Source_span span_of(If_expression const &node);
  Source_span span_of(Constructor_expression const &node);

  Source_span span_of(Statement const &node);
  Source_span span_of(Let_statement const &node);
  Source_span span_of(While_statement const &node);
  Source_span span_of(Return_statement const &node);
  Source_span span_of(Expression_statement const &node);
  Source_span span_of(Function_definition const &node);

} // namespace basedparse

#endif // BASEDPARSE_SOURCE_SPAN_H
