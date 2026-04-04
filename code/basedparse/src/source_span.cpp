#include <variant>

#include "basedparse/source_span.h"

namespace basedparse
{

  Source_span span_of(basedlex::Lexeme const &lexeme)
  {
    auto const end = Source_location{
      .line = lexeme.location.line,
      .column = lexeme.location.column +
                static_cast<std::int32_t>(lexeme.text.size()) - 1,
    };
    return Source_span{.start = lexeme.location, .end = end};
  }

  Source_span hull(Source_span begin, Source_span end)
  {
    return Source_span{.start = begin.start, .end = end.end};
  }

  // expressions

  Source_span span_of(Expression const &node)
  {
    return std::visit(
      [&](auto const &n) -> Source_span
      {
        return span_of(n);
      },
      node.value
    );
  }

  Source_span span_of(Int_literal_expression const &node)
  {
    return span_of(node.literal);
  }

  Source_span span_of(Identifier_expression const &node)
  {
    return span_of(node.identifier);
  }

  Source_span span_of(Recurse_expression const &node)
  {
    return span_of(node.kw_recurse);
  }

  Source_span span_of(Fn_expression const &node)
  {
    return hull(span_of(node.kw_fn), span_of(*node.body));
  }

  Source_span span_of(Paren_expression const &node)
  {
    return hull(span_of(node.lparen), span_of(node.rparen));
  }

  Source_span span_of(Prefix_expression const &node)
  {
    return hull(span_of(node.op), span_of(*node.operand));
  }

  Source_span span_of(Postfix_expression const &node)
  {
    return hull(span_of(*node.operand), span_of(node.op));
  }

  Source_span span_of(Binary_expression const &node)
  {
    return hull(span_of(*node.left), span_of(*node.right));
  }

  Source_span span_of(Call_expression const &node)
  {
    return hull(span_of(*node.callee), span_of(node.rparen));
  }

  Source_span span_of(Index_expression const &node)
  {
    return hull(span_of(*node.operand), span_of(node.rbracket));
  }

  Source_span span_of(Prefix_bracket_expression const &node)
  {
    return hull(span_of(node.lbracket), span_of(*node.operand));
  }

  Source_span span_of(Block_expression const &node)
  {
    return hull(span_of(node.lbrace), span_of(node.rbrace));
  }

  Source_span span_of(If_expression const &node)
  {
    auto const end_span = [&]() -> Source_span
    {
      if (node.else_part.has_value())
      {
        return span_of(node.else_part->body.rbrace);
      }
      if (!node.else_if_parts.empty())
      {
        return span_of(node.else_if_parts.back().body.rbrace);
      }
      return span_of(node.then_block.rbrace);
    }();
    return hull(span_of(node.kw_if), end_span);
  }

  // statements

  Source_span span_of(Statement const &node)
  {
    return std::visit(
      [&](auto const &n) -> Source_span
      {
        return span_of(n);
      },
      node.value
    );
  }

  Source_span span_of(Let_statement const &node)
  {
    return hull(span_of(node.kw_let), span_of(node.semicolon));
  }

  Source_span span_of(While_statement const &node)
  {
    return hull(span_of(node.kw_while), span_of(node.body.rbrace));
  }

  Source_span span_of(Return_statement const &node)
  {
    return hull(span_of(node.kw_return), span_of(node.semicolon));
  }

  Source_span span_of(Expression_statement const &node)
  {
    return hull(span_of(node.expression), span_of(node.semicolon));
  }

} // namespace basedparse
