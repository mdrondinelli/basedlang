/// @file
/// @brief Hand-written recursive descent parser.
///
/// Each grammar construct has a corresponding parse_* method. The parser reads
/// from a Lexeme_stream_reader, which provides arbitrary lookahead via peek()
/// and consumes tokens via read(). expect() is a helper that reads and asserts
/// a specific token, throwing on mismatch.
///
/// See grammar.md for the full grammar.

#include <limits>
#include <stdexcept>
#include <string>

#include <bensonast/operator.h>

#include "bensonparse/parser.h"

namespace benson
{

  Parser::Parser(benson::Lexeme_stream_reader *reader) noexcept
      : _reader{reader}
  {
  }

  benson::ast::Translation_unit Parser::parse_translation_unit()
  {
    auto unit = benson::ast::Translation_unit{};
    while (_reader->peek().token != benson::Token::eof)
    {
      unit.let_statements.push_back(parse_let_statement());
    }
    return unit;
  }

  benson::ast::Statement Parser::parse_statement()
  {
    auto const &next = _reader->peek();
    if (next.token == benson::Token::kw_let)
    {
      return benson::ast::Statement{parse_let_statement()};
    }
    if (next.token == benson::Token::kw_return)
    {
      return benson::ast::Statement{parse_return_statement()};
    }
    if (next.token == benson::Token::kw_while)
    {
      return benson::ast::Statement{parse_while_statement()};
    }
    return benson::ast::Statement{parse_expression_statement()};
  }

  benson::ast::Let_statement Parser::parse_let_statement()
  {
    auto stmt = benson::ast::Let_statement{};
    stmt.kw_let = expect(benson::Token::kw_let);
    if (_reader->peek().token == benson::Token::kw_mut)
    {
      stmt.kw_mut = expect(benson::Token::kw_mut);
    }
    stmt.name = expect(benson::Token::identifier);
    stmt.eq = expect(benson::Token::eq);
    stmt.initializer = std::move(*parse_expression());
    stmt.semicolon = expect(benson::Token::semicolon);
    return stmt;
  }

  benson::ast::Return_statement Parser::parse_return_statement()
  {
    auto stmt = benson::ast::Return_statement{};
    stmt.kw_return = expect(benson::Token::kw_return);
    stmt.value = std::move(*parse_expression());
    stmt.semicolon = expect(benson::Token::semicolon);
    return stmt;
  }

  benson::ast::Expression_statement Parser::parse_expression_statement()
  {
    auto stmt = benson::ast::Expression_statement{};
    stmt.expression = std::move(*parse_expression());
    stmt.semicolon = expect(benson::Token::semicolon);
    return stmt;
  }

  benson::ast::Block_expression Parser::parse_block_expression()
  {
    auto block = benson::ast::Block_expression{};
    block.lbrace = expect(benson::Token::lbrace);
    while (_reader->peek().token != benson::Token::rbrace)
    {
      auto const &next = _reader->peek();
      if (next.token == benson::Token::kw_let)
      {
        block.statements.push_back(benson::ast::Statement{parse_let_statement()});
      }
      else if (next.token == benson::Token::kw_return)
      {
        block.statements.push_back(
          benson::ast::Statement{parse_return_statement()}
        );
      }
      else if (next.token == benson::Token::kw_while)
      {
        block.statements.push_back(
          benson::ast::Statement{parse_while_statement()}
        );
      }
      else
      {
        auto expr = parse_expression();
        if (_reader->peek().token == benson::Token::semicolon)
        {
          auto stmt = benson::ast::Expression_statement{};
          stmt.expression = std::move(*expr);
          stmt.semicolon = expect(benson::Token::semicolon);
          block.statements.push_back(benson::ast::Statement{std::move(stmt)});
        }
        else
        {
          block.tail = std::move(expr);
          break;
        }
      }
    }
    block.rbrace = expect(benson::Token::rbrace);
    return block;
  }

  std::unique_ptr<benson::ast::Expression> Parser::parse_expression()
  {
    return parse_expression(std::numeric_limits<int>::max());
  }

  /// @brief Parse an expression using Pratt (top-down operator precedence)
  /// parsing.
  ///
  /// @param current_precedence Only consume operators whose precedence is <=
  ///   this value. The public overload passes INT_MAX to allow all operators.
  ///   Recursive calls pass `op_prec - 1` for left-associative binary ops or
  ///   `op_prec` for right-associative ops. Associativity is per-level, queried
  ///   via `get_precedence_associativity`.
  ///
  /// The algorithm works in three phases:
  ///
  /// 1. **Prefix**: If the next token is a prefix operator (e.g. `-`, `^`),
  ///    consume it and recursively parse the operand at a tighter precedence
  ///    (one below the prefix op's level). Otherwise fall through to primary.
  ///
  /// 2. **Postfix**: Repeatedly consume postfix operators (call `()`, index
  /// `[]`,
  ///    dereference `^`) that bind tighter than the current precedence level.
  ///
  /// 3. **Binary infix**: Repeatedly consume binary operators (e.g. `+`, `<`,
  ///    `==`) whose precedence fits within `current_precedence`. For each one,
  ///    the right-hand side is parsed by recursing with `op_prec - 1`, which
  ///    enforces left-associativity (operators at the same level fold left).
  ///
  /// **Precedence table** (lower number = binds tighter):
  /// - 0: postfix call `()`, index `[]`, dereference `^`
  /// - 1: prefix `&` (address-of), `^` (pointer-to), unary `+`, unary `-`
  /// - 2: `*`, `/`, `%`
  /// - 3: `+`, `-`
  /// - 4: `<`, `<=`, `>`, `>=`
  /// - 5: `==`, `!=`
  /// - 6: `=` (right-associative)
  ///
  /// **To add a new operator:**
  /// 1. Add a token to `benson::Token` (token.h) and lex it in
  /// lexeme_stream.cpp.
  /// 2. Add a variant to `benson::ast::Operator` (operator.h).
  /// 3. Assign it a precedence in `get_operator_precedence` (operator.cpp).
  /// 4. Map the token to the operator in the appropriate get_*_operator
  /// function
  ///    (operator.cpp): `get_prefix_operator`, `get_postfix_operator`, or
  ///    `get_binary_operator`. No changes to the parser itself are needed.
  std::unique_ptr<benson::ast::Expression>
  Parser::parse_expression(int current_precedence)
  {
    auto expr = [&]() -> std::unique_ptr<benson::ast::Expression>
    {
      if (auto const prefix_op =
            benson::ast::get_prefix_operator(_reader->peek().token);
          prefix_op &&
          benson::ast::get_operator_precedence(*prefix_op) <= current_precedence)
      {
        auto prefix = benson::ast::Prefix_expression{};
        prefix.op = _reader->read();
        prefix.operand =
          parse_expression(benson::ast::get_operator_precedence(*prefix_op));
        return std::make_unique<benson::ast::Expression>(std::move(prefix));
      }
      if (_reader->peek().token == benson::Token::lbracket &&
          1 <= current_precedence)
      {
        auto prefix = benson::ast::Prefix_bracket_expression{};
        prefix.lbracket = expect(benson::Token::lbracket);
        if (_reader->peek().token != benson::Token::rbracket)
        {
          prefix.size = parse_expression();
        }
        prefix.rbracket = expect(benson::Token::rbracket);
        prefix.operand = parse_expression(1);
        return std::make_unique<benson::ast::Expression>(std::move(prefix));
      }
      auto primary = parse_primary_expression();
      for (;;)
      {
        if (auto const postfix_op =
              benson::ast::get_postfix_operator(_reader->peek().token);
            postfix_op && benson::ast::get_operator_precedence(*postfix_op) <=
                            current_precedence)
        {
          if (*postfix_op == benson::ast::Operator::call)
          {
            auto call = benson::ast::Call_expression{};
            call.callee = std::move(primary);
            call.lparen = expect(benson::Token::lparen);
            for (;;)
            {
              if (_reader->peek().token == benson::Token::rparen)
              {
                break;
              }
              call.arguments.push_back(std::move(*parse_expression()));
              if (_reader->peek().token != benson::Token::comma)
              {
                break;
              }
              call.argument_commas.push_back(expect(benson::Token::comma));
            }
            call.rparen = expect(benson::Token::rparen);
            primary = std::make_unique<benson::ast::Expression>(std::move(call));
          }
          else if (*postfix_op == benson::ast::Operator::index)
          {
            auto idx = benson::ast::Index_expression{};
            idx.operand = std::move(primary);
            idx.lbracket = expect(benson::Token::lbracket);
            idx.index = parse_expression();
            idx.rbracket = expect(benson::Token::rbracket);
            primary = std::make_unique<benson::ast::Expression>(std::move(idx));
          }
          else
          {
            auto postfix = benson::ast::Postfix_expression{};
            postfix.operand = std::move(primary);
            postfix.op = _reader->read();
            primary =
              std::make_unique<benson::ast::Expression>(std::move(postfix));
          }
        }
        else
        {
          break;
        }
      }
      return primary;
    }();
    for (;;)
    {
      if (auto const bin_op =
            benson::ast::get_binary_operator(_reader->peek().token);
          bin_op &&
          benson::ast::get_operator_precedence(*bin_op) <= current_precedence)
      {
        auto const op_prec = benson::ast::get_operator_precedence(*bin_op);
        auto const rhs_prec = benson::ast::get_precedence_associativity(op_prec) ==
                                  benson::ast::Operator_associativity::right
                                ? op_prec
                                : op_prec - 1;
        auto binary = benson::ast::Binary_expression{};
        binary.left = std::move(expr);
        binary.op = _reader->read();
        binary.right = parse_expression(rhs_prec);
        expr = std::make_unique<benson::ast::Expression>(std::move(binary));
      }
      else
      {
        break;
      }
    }
    return expr;
  }

  std::unique_ptr<benson::ast::Expression> Parser::parse_primary_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == benson::Token::int_literal)
    {
      return std::make_unique<benson::ast::Expression>(
        parse_int_literal_expression()
      );
    }
    if (next.token == benson::Token::float_literal)
    {
      return std::make_unique<benson::ast::Expression>(
        parse_float_literal_expression()
      );
    }
    if (next.token == benson::Token::identifier)
    {
      return std::make_unique<benson::ast::Expression>(
        parse_identifier_expression()
      );
    }
    if (next.token == benson::Token::kw_recurse)
    {
      return std::make_unique<benson::ast::Expression>(
        benson::ast::Recurse_expression{.kw_recurse = _reader->read()}
      );
    }
    if (next.token == benson::Token::kw_if)
    {
      return std::make_unique<benson::ast::Expression>(parse_if_expression());
    }
    if (next.token == benson::Token::lbrace)
    {
      return std::make_unique<benson::ast::Expression>(parse_block_expression());
    }
    if (next.token == benson::Token::lparen)
    {
      return std::make_unique<benson::ast::Expression>(parse_paren_expression());
    }
    if (next.token == benson::Token::kw_fn)
    {
      return std::make_unique<benson::ast::Expression>(parse_fn_expression());
    }
    throw std::runtime_error{
      "unexpected token '" + next.text + "' at " +
      std::to_string(next.location.line) + ":" +
      std::to_string(next.location.column)
    };
  }

  benson::ast::Fn_expression Parser::parse_fn_expression()
  {
    auto fn = benson::ast::Fn_expression{};
    fn.kw_fn = expect(benson::Token::kw_fn);
    fn.lparen = expect(benson::Token::lparen);
    for (;;)
    {
      if (_reader->peek().token == benson::Token::rparen)
      {
        break;
      }
      auto param = benson::ast::Fn_expression::Parameter_declaration{};
      if (_reader->peek().token == benson::Token::kw_mut)
      {
        param.kw_mut = expect(benson::Token::kw_mut);
      }
      param.name = expect(benson::Token::identifier);
      param.colon = expect(benson::Token::colon);
      param.type = parse_expression();
      fn.parameters.push_back(std::move(param));
      if (_reader->peek().token != benson::Token::comma)
      {
        break;
      }
      fn.parameter_commas.push_back(expect(benson::Token::comma));
    }
    fn.rparen = expect(benson::Token::rparen);
    fn.return_type_specifier = parse_return_type_specifier();
    fn.arrow = expect(benson::Token::fat_arrow);
    fn.body = parse_expression();
    return fn;
  }

  benson::ast::Fn_expression::Return_type_specifier
  Parser::parse_return_type_specifier()
  {
    auto spec = benson::ast::Fn_expression::Return_type_specifier{};
    spec.colon = expect(benson::Token::colon);
    spec.type = parse_expression();
    return spec;
  }

  benson::ast::Int_literal_expression Parser::parse_int_literal_expression()
  {
    return benson::ast::Int_literal_expression{
      .literal = expect(benson::Token::int_literal)
    };
  }

  benson::ast::Float_literal_expression Parser::parse_float_literal_expression()
  {
    return benson::ast::Float_literal_expression{
      .literal = expect(benson::Token::float_literal)
    };
  }

  benson::ast::Identifier_expression Parser::parse_identifier_expression()
  {
    return benson::ast::Identifier_expression{
      .identifier = expect(benson::Token::identifier)
    };
  }

  benson::ast::Paren_expression Parser::parse_paren_expression()
  {
    auto expr = benson::ast::Paren_expression{};
    expr.lparen = expect(benson::Token::lparen);
    expr.inner = parse_expression();
    expr.rparen = expect(benson::Token::rparen);
    return expr;
  }

  benson::ast::If_expression Parser::parse_if_expression()
  {
    auto expr = benson::ast::If_expression{};
    expr.kw_if = expect(benson::Token::kw_if);
    expr.condition = parse_expression();
    expr.then_block = parse_block_expression();
    while (_reader->peek().token == benson::Token::kw_else)
    {
      auto kw_else = expect(benson::Token::kw_else);
      if (_reader->peek().token == benson::Token::kw_if)
      {
        auto part = benson::ast::If_expression::Else_if_part{};
        part.kw_else = kw_else;
        part.kw_if = expect(benson::Token::kw_if);
        part.condition = parse_expression();
        part.body = parse_block_expression();
        expr.else_if_parts.push_back(std::move(part));
      }
      else
      {
        auto else_part = benson::ast::If_expression::Else_part{};
        else_part.kw_else = kw_else;
        else_part.body = parse_block_expression();
        expr.else_part = std::move(else_part);
        break;
      }
    }
    return expr;
  }

  benson::ast::While_statement Parser::parse_while_statement()
  {
    auto stmt = benson::ast::While_statement{};
    stmt.kw_while = expect(benson::Token::kw_while);
    stmt.condition = parse_expression();
    stmt.body = parse_block_expression();
    return stmt;
  }

  benson::Lexeme Parser::expect(benson::Token token)
  {
    auto lexeme = _reader->read();
    if (lexeme.token != token)
    {
      throw std::runtime_error{
        "unexpected token '" + lexeme.text + "' at " +
        std::to_string(lexeme.location.line) + ":" +
        std::to_string(lexeme.location.column)
      };
    }
    return lexeme;
  }

} // namespace benson
