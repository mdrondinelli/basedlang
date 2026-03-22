#include <variant>

#include "basedparse/source_span.h"
#include "basedlex/lexeme.h"

namespace basedparse
{

  namespace
  {
    Source_location location_of(basedlex::Lexeme const &lexeme)
    {
      return Source_location{.line = lexeme.line, .column = lexeme.column};
    }
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
    auto const loc = location_of(node.identifier);
    return Source_span{.start = loc, .end = loc};
  }

  Source_span source_span(Array_type_expression const &node)
  {
    return Source_span{
      .start = source_span(*node.element_type).start,
      .end = location_of(node.rbracket),
    };
  }

  Source_span source_span(Pointer_type_expression const &node)
  {
    return Source_span{
      .start = source_span(*node.pointee_type).start,
      .end = location_of(node.star),
    };
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
    auto const loc = location_of(node.literal);
    return Source_span{.start = loc, .end = loc};
  }

  Source_span source_span(Identifier_expression const &node)
  {
    auto const loc = location_of(node.identifier);
    return Source_span{.start = loc, .end = loc};
  }

  Source_span source_span(Fn_expression const &node)
  {
    return Source_span{
      .start = location_of(node.kw_fn),
      .end = source_span(*node.body).end,
    };
  }

  Source_span source_span(Paren_expression const &node)
  {
    return Source_span{
      .start = location_of(node.lparen),
      .end = location_of(node.rparen),
    };
  }

  Source_span source_span(Unary_expression const &node)
  {
    return Source_span{
      .start = location_of(node.op),
      .end = source_span(*node.operand).end,
    };
  }

  Source_span source_span(Binary_expression const &node)
  {
    return Source_span{
      .start = source_span(*node.left).start,
      .end = source_span(*node.right).end,
    };
  }

  Source_span source_span(Call_expression const &node)
  {
    return Source_span{
      .start = source_span(*node.callee).start,
      .end = location_of(node.rparen),
    };
  }

  Source_span source_span(Index_expression const &node)
  {
    return Source_span{
      .start = source_span(*node.operand).start,
      .end = location_of(node.rbracket),
    };
  }

  Source_span source_span(Block_expression const &node)
  {
    return Source_span{
      .start = location_of(node.lbrace),
      .end = location_of(node.rbrace),
    };
  }

  Source_span source_span(If_expression const &node)
  {
    auto end = Source_location{};
    if (node.else_part.has_value())
    {
      end = location_of(node.else_part->body.rbrace);
    }
    else if (!node.else_if_parts.empty())
    {
      end = location_of(node.else_if_parts.back().body.rbrace);
    }
    else
    {
      end = location_of(node.then_block.rbrace);
    }
    return Source_span{
      .start = location_of(node.kw_if),
      .end = end,
    };
  }

  Source_span source_span(Constructor_expression const &node)
  {
    return Source_span{
      .start = location_of(node.kw_new),
      .end = location_of(node.rbrace),
    };
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
    return Source_span{
      .start = location_of(node.kw_let),
      .end = location_of(node.semicolon),
    };
  }

  Source_span source_span(While_statement const &node)
  {
    return Source_span{
      .start = location_of(node.kw_while),
      .end = location_of(node.body.rbrace),
    };
  }

  Source_span source_span(Return_statement const &node)
  {
    return Source_span{
      .start = location_of(node.kw_return),
      .end = location_of(node.semicolon),
    };
  }

  Source_span source_span(Expression_statement const &node)
  {
    return Source_span{
      .start = source_span(node.expression).start,
      .end = location_of(node.semicolon),
    };
  }

  Source_span source_span(Function_definition const &node)
  {
    return Source_span{
      .start = location_of(node.kw_let),
      .end = location_of(node.semicolon),
    };
  }

} // namespace basedparse
