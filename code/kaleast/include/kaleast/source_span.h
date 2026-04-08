#ifndef BASEDAST_SOURCE_SPAN_H
#define BASEDAST_SOURCE_SPAN_H

#include "ast.h"
#include "source_location.h"

namespace kaleast
{

  /// A closed range of source positions [start, end].
  /// Both @c start and @c end are inclusive: @c start is the position of the
  /// first character and @c end is the position of the last character.
  struct Source_span
  {
    Source_location start; ///< Position of the first character (inclusive).
    Source_location end; ///< Position of the last character (inclusive).
  };

  /// Returns the source span of a lexeme.
  /// @c start points to the first character of the lexeme text; @c end points
  /// to the last character.
  Source_span span_of(kalelex::Lexeme const &lexeme);

  /// Returns the smallest span whose @c start comes from @p begin and whose
  /// @c end comes from @p end.  Use this to build the span of a composite
  /// construct from the spans of its outermost tokens or sub-nodes.
  Source_span hull(Source_span begin, Source_span end);

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

} // namespace kaleast

#endif // BASEDAST_SOURCE_SPAN_H
