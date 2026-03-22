#include <variant>

#include "basedparse/source_span.h"

namespace basedparse
{

  Source_span lexeme_span(basedlex::Lexeme const &lexeme)
  {
    auto const begin = Source_location{
      .line = lexeme.line,
      .column = lexeme.column,
    };
    auto const end = Source_location{
      .line = lexeme.line,
      .column = lexeme.column + static_cast<std::int32_t>(lexeme.text.size()),
    };
    return Source_span{.start = begin, .end = end};
  }

  Source_span hull(Source_span begin, Source_span end)
  {
    return Source_span{.start = begin.start, .end = end.end};
  }


  // type expressions

  Source_span source_span(Type_expression const &node)
  {
    return std::visit(
      [&](auto const &n) -> Source_span
      {
        return source_span(n);
      },
      node.value
    );
  }

  Source_span source_span(Identifier_type_expression const &node)
  {
    return lexeme_span(node.identifier);
  }

  Source_span source_span(Array_type_expression const &node)
  {
    return hull(source_span(*node.element_type), lexeme_span(node.rbracket));
  }

  Source_span source_span(Pointer_type_expression const &node)
  {
    return hull(source_span(*node.pointee_type), lexeme_span(node.star));
  }

  // expressions

  Source_span source_span(Expression const &node)
  {
    return std::visit(
      [&](auto const &n) -> Source_span
      {
        return source_span(n);
      },
      node.value
    );
  }

  Source_span source_span(Int_literal_expression const &node)
  {
    return lexeme_span(node.literal);
  }

  Source_span source_span(Identifier_expression const &node)
  {
    return lexeme_span(node.identifier);
  }

  Source_span source_span(Fn_expression const &node)
  {
    return hull(lexeme_span(node.kw_fn), source_span(*node.body));
  }

  Source_span source_span(Paren_expression const &node)
  {
    return hull(lexeme_span(node.lparen), lexeme_span(node.rparen));
  }

  Source_span source_span(Unary_expression const &node)
  {
    return hull(lexeme_span(node.op), source_span(*node.operand));
  }

  Source_span source_span(Binary_expression const &node)
  {
    return hull(source_span(*node.left), source_span(*node.right));
  }

  Source_span source_span(Call_expression const &node)
  {
    return hull(source_span(*node.callee), lexeme_span(node.rparen));
  }

  Source_span source_span(Index_expression const &node)
  {
    return hull(source_span(*node.operand), lexeme_span(node.rbracket));
  }

  Source_span source_span(Block_expression const &node)
  {
    return hull(lexeme_span(node.lbrace), lexeme_span(node.rbrace));
  }

  Source_span source_span(If_expression const &node)
  {
    auto const end_span = [&]() -> Source_span
    {
      if (node.else_part.has_value())
      {
        return lexeme_span(node.else_part->body.rbrace);
      }
      if (!node.else_if_parts.empty())
      {
        return lexeme_span(node.else_if_parts.back().body.rbrace);
      }
      return lexeme_span(node.then_block.rbrace);
    }();
    return hull(lexeme_span(node.kw_if), end_span);
  }

  Source_span source_span(Constructor_expression const &node)
  {
    return hull(lexeme_span(node.kw_new), lexeme_span(node.rbrace));
  }

  // statements

  Source_span source_span(Statement const &node)
  {
    return std::visit(
      [&](auto const &n) -> Source_span
      {
        return source_span(n);
      },
      node.value
    );
  }

  Source_span source_span(Let_statement const &node)
  {
    return hull(lexeme_span(node.kw_let), lexeme_span(node.semicolon));
  }

  Source_span source_span(While_statement const &node)
  {
    return hull(lexeme_span(node.kw_while), lexeme_span(node.body.rbrace));
  }

  Source_span source_span(Return_statement const &node)
  {
    return hull(lexeme_span(node.kw_return), lexeme_span(node.semicolon));
  }

  Source_span source_span(Expression_statement const &node)
  {
    return hull(source_span(node.expression), lexeme_span(node.semicolon));
  }

  Source_span source_span(Function_definition const &node)
  {
    return hull(lexeme_span(node.kw_let), lexeme_span(node.semicolon));
  }

} // namespace basedparse
