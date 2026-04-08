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

#include <kaleast/operator.h>

#include "kaleparse/parser.h"

namespace kaleparse
{

  Parser::Parser(kalelex::Lexeme_stream_reader *reader) noexcept
      : _reader{reader}
  {
  }

  kaleast::Translation_unit Parser::parse_translation_unit()
  {
    auto unit = kaleast::Translation_unit{};
    while (_reader->peek().token != kalelex::Token::eof)
    {
      unit.let_statements.push_back(parse_let_statement());
    }
    return unit;
  }

  kaleast::Statement Parser::parse_statement()
  {
    auto const &next = _reader->peek();
    if (next.token == kalelex::Token::kw_let)
    {
      return kaleast::Statement{parse_let_statement()};
    }
    if (next.token == kalelex::Token::kw_return)
    {
      return kaleast::Statement{parse_return_statement()};
    }
    if (next.token == kalelex::Token::kw_while)
    {
      return kaleast::Statement{parse_while_statement()};
    }
    return kaleast::Statement{parse_expression_statement()};
  }

  kaleast::Let_statement Parser::parse_let_statement()
  {
    auto stmt = kaleast::Let_statement{};
    stmt.kw_let = expect(kalelex::Token::kw_let);
    if (_reader->peek().token == kalelex::Token::kw_mut)
    {
      stmt.kw_mut = expect(kalelex::Token::kw_mut);
    }
    stmt.name = expect(kalelex::Token::identifier);
    stmt.eq = expect(kalelex::Token::eq);
    stmt.initializer = std::move(*parse_expression());
    stmt.semicolon = expect(kalelex::Token::semicolon);
    return stmt;
  }

  kaleast::Return_statement Parser::parse_return_statement()
  {
    auto stmt = kaleast::Return_statement{};
    stmt.kw_return = expect(kalelex::Token::kw_return);
    stmt.value = std::move(*parse_expression());
    stmt.semicolon = expect(kalelex::Token::semicolon);
    return stmt;
  }

  kaleast::Expression_statement Parser::parse_expression_statement()
  {
    auto stmt = kaleast::Expression_statement{};
    stmt.expression = std::move(*parse_expression());
    stmt.semicolon = expect(kalelex::Token::semicolon);
    return stmt;
  }

  kaleast::Block_expression Parser::parse_block_expression()
  {
    auto block = kaleast::Block_expression{};
    block.lbrace = expect(kalelex::Token::lbrace);
    while (_reader->peek().token != kalelex::Token::rbrace)
    {
      auto const &next = _reader->peek();
      if (next.token == kalelex::Token::kw_let)
      {
        block.statements.push_back(kaleast::Statement{parse_let_statement()});
      }
      else if (next.token == kalelex::Token::kw_return)
      {
        block.statements.push_back(
          kaleast::Statement{parse_return_statement()}
        );
      }
      else if (next.token == kalelex::Token::kw_while)
      {
        block.statements.push_back(
          kaleast::Statement{parse_while_statement()}
        );
      }
      else
      {
        auto expr = parse_expression();
        if (_reader->peek().token == kalelex::Token::semicolon)
        {
          auto stmt = kaleast::Expression_statement{};
          stmt.expression = std::move(*expr);
          stmt.semicolon = expect(kalelex::Token::semicolon);
          block.statements.push_back(kaleast::Statement{std::move(stmt)});
        }
        else
        {
          block.tail = std::move(expr);
          break;
        }
      }
    }
    block.rbrace = expect(kalelex::Token::rbrace);
    return block;
  }

  std::unique_ptr<kaleast::Expression> Parser::parse_expression()
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
  /// 1. Add a token to `kalelex::Token` (token.h) and lex it in
  /// lexeme_stream.cpp.
  /// 2. Add a variant to `kaleast::Operator` (operator.h).
  /// 3. Assign it a precedence in `get_operator_precedence` (operator.cpp).
  /// 4. Map the token to the operator in the appropriate get_*_operator
  /// function
  ///    (operator.cpp): `get_prefix_operator`, `get_postfix_operator`, or
  ///    `get_binary_operator`. No changes to the parser itself are needed.
  std::unique_ptr<kaleast::Expression>
  Parser::parse_expression(int current_precedence)
  {
    auto expr = [&]() -> std::unique_ptr<kaleast::Expression>
    {
      if (auto const prefix_op =
            kaleast::get_prefix_operator(_reader->peek().token);
          prefix_op &&
          kaleast::get_operator_precedence(*prefix_op) <= current_precedence)
      {
        auto prefix = kaleast::Prefix_expression{};
        prefix.op = _reader->read();
        prefix.operand =
          parse_expression(kaleast::get_operator_precedence(*prefix_op));
        return std::make_unique<kaleast::Expression>(std::move(prefix));
      }
      if (_reader->peek().token == kalelex::Token::lbracket &&
          1 <= current_precedence)
      {
        auto prefix = kaleast::Prefix_bracket_expression{};
        prefix.lbracket = expect(kalelex::Token::lbracket);
        if (_reader->peek().token != kalelex::Token::rbracket)
        {
          prefix.size = parse_expression();
        }
        prefix.rbracket = expect(kalelex::Token::rbracket);
        prefix.operand = parse_expression(1);
        return std::make_unique<kaleast::Expression>(std::move(prefix));
      }
      auto primary = parse_primary_expression();
      for (;;)
      {
        if (auto const postfix_op =
              kaleast::get_postfix_operator(_reader->peek().token);
            postfix_op && kaleast::get_operator_precedence(*postfix_op) <=
                            current_precedence)
        {
          if (*postfix_op == kaleast::Operator::call)
          {
            auto call = kaleast::Call_expression{};
            call.callee = std::move(primary);
            call.lparen = expect(kalelex::Token::lparen);
            for (;;)
            {
              if (_reader->peek().token == kalelex::Token::rparen)
              {
                break;
              }
              call.arguments.push_back(std::move(*parse_expression()));
              if (_reader->peek().token != kalelex::Token::comma)
              {
                break;
              }
              call.argument_commas.push_back(expect(kalelex::Token::comma));
            }
            call.rparen = expect(kalelex::Token::rparen);
            primary = std::make_unique<kaleast::Expression>(std::move(call));
          }
          else if (*postfix_op == kaleast::Operator::index)
          {
            auto idx = kaleast::Index_expression{};
            idx.operand = std::move(primary);
            idx.lbracket = expect(kalelex::Token::lbracket);
            idx.index = parse_expression();
            idx.rbracket = expect(kalelex::Token::rbracket);
            primary = std::make_unique<kaleast::Expression>(std::move(idx));
          }
          else
          {
            auto postfix = kaleast::Postfix_expression{};
            postfix.operand = std::move(primary);
            postfix.op = _reader->read();
            primary =
              std::make_unique<kaleast::Expression>(std::move(postfix));
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
            kaleast::get_binary_operator(_reader->peek().token);
          bin_op &&
          kaleast::get_operator_precedence(*bin_op) <= current_precedence)
      {
        auto const op_prec = kaleast::get_operator_precedence(*bin_op);
        auto const rhs_prec = kaleast::get_precedence_associativity(op_prec) ==
                                  kaleast::Operator_associativity::right
                                ? op_prec
                                : op_prec - 1;
        auto binary = kaleast::Binary_expression{};
        binary.left = std::move(expr);
        binary.op = _reader->read();
        binary.right = parse_expression(rhs_prec);
        expr = std::make_unique<kaleast::Expression>(std::move(binary));
      }
      else
      {
        break;
      }
    }
    return expr;
  }

  std::unique_ptr<kaleast::Expression> Parser::parse_primary_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == kalelex::Token::int_literal)
    {
      return std::make_unique<kaleast::Expression>(
        parse_int_literal_expression()
      );
    }
    if (next.token == kalelex::Token::float_literal)
    {
      return std::make_unique<kaleast::Expression>(
        parse_float_literal_expression()
      );
    }
    if (next.token == kalelex::Token::identifier)
    {
      return std::make_unique<kaleast::Expression>(
        parse_identifier_expression()
      );
    }
    if (next.token == kalelex::Token::kw_recurse)
    {
      return std::make_unique<kaleast::Expression>(
        kaleast::Recurse_expression{.kw_recurse = _reader->read()}
      );
    }
    if (next.token == kalelex::Token::kw_if)
    {
      return std::make_unique<kaleast::Expression>(parse_if_expression());
    }
    if (next.token == kalelex::Token::lbrace)
    {
      return std::make_unique<kaleast::Expression>(parse_block_expression());
    }
    if (next.token == kalelex::Token::lparen)
    {
      return std::make_unique<kaleast::Expression>(parse_paren_expression());
    }
    if (next.token == kalelex::Token::kw_fn)
    {
      return std::make_unique<kaleast::Expression>(parse_fn_expression());
    }
    throw std::runtime_error{
      "unexpected token '" + next.text + "' at " +
      std::to_string(next.location.line) + ":" +
      std::to_string(next.location.column)
    };
  }

  kaleast::Fn_expression Parser::parse_fn_expression()
  {
    auto fn = kaleast::Fn_expression{};
    fn.kw_fn = expect(kalelex::Token::kw_fn);
    fn.lparen = expect(kalelex::Token::lparen);
    for (;;)
    {
      if (_reader->peek().token == kalelex::Token::rparen)
      {
        break;
      }
      auto param = kaleast::Fn_expression::Parameter_declaration{};
      if (_reader->peek().token == kalelex::Token::kw_mut)
      {
        param.kw_mut = expect(kalelex::Token::kw_mut);
      }
      param.name = expect(kalelex::Token::identifier);
      param.colon = expect(kalelex::Token::colon);
      param.type = parse_expression();
      fn.parameters.push_back(std::move(param));
      if (_reader->peek().token != kalelex::Token::comma)
      {
        break;
      }
      fn.parameter_commas.push_back(expect(kalelex::Token::comma));
    }
    fn.rparen = expect(kalelex::Token::rparen);
    fn.return_type_specifier = parse_return_type_specifier();
    fn.arrow = expect(kalelex::Token::fat_arrow);
    fn.body = parse_expression();
    return fn;
  }

  kaleast::Fn_expression::Return_type_specifier
  Parser::parse_return_type_specifier()
  {
    auto spec = kaleast::Fn_expression::Return_type_specifier{};
    spec.colon = expect(kalelex::Token::colon);
    spec.type = parse_expression();
    return spec;
  }

  kaleast::Int_literal_expression Parser::parse_int_literal_expression()
  {
    return kaleast::Int_literal_expression{
      .literal = expect(kalelex::Token::int_literal)
    };
  }

  kaleast::Float_literal_expression Parser::parse_float_literal_expression()
  {
    return kaleast::Float_literal_expression{
      .literal = expect(kalelex::Token::float_literal)
    };
  }

  kaleast::Identifier_expression Parser::parse_identifier_expression()
  {
    return kaleast::Identifier_expression{
      .identifier = expect(kalelex::Token::identifier)
    };
  }

  kaleast::Paren_expression Parser::parse_paren_expression()
  {
    auto expr = kaleast::Paren_expression{};
    expr.lparen = expect(kalelex::Token::lparen);
    expr.inner = parse_expression();
    expr.rparen = expect(kalelex::Token::rparen);
    return expr;
  }

  kaleast::If_expression Parser::parse_if_expression()
  {
    auto expr = kaleast::If_expression{};
    expr.kw_if = expect(kalelex::Token::kw_if);
    expr.condition = parse_expression();
    expr.then_block = parse_block_expression();
    while (_reader->peek().token == kalelex::Token::kw_else)
    {
      auto kw_else = expect(kalelex::Token::kw_else);
      if (_reader->peek().token == kalelex::Token::kw_if)
      {
        auto part = kaleast::If_expression::Else_if_part{};
        part.kw_else = kw_else;
        part.kw_if = expect(kalelex::Token::kw_if);
        part.condition = parse_expression();
        part.body = parse_block_expression();
        expr.else_if_parts.push_back(std::move(part));
      }
      else
      {
        auto else_part = kaleast::If_expression::Else_part{};
        else_part.kw_else = kw_else;
        else_part.body = parse_block_expression();
        expr.else_part = std::move(else_part);
        break;
      }
    }
    return expr;
  }

  kaleast::While_statement Parser::parse_while_statement()
  {
    auto stmt = kaleast::While_statement{};
    stmt.kw_while = expect(kalelex::Token::kw_while);
    stmt.condition = parse_expression();
    stmt.body = parse_block_expression();
    return stmt;
  }

  kalelex::Lexeme Parser::expect(kalelex::Token token)
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

} // namespace kaleparse
