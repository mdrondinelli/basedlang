/// @file
/// @brief Hand-written recursive descent parser.
///
/// Each grammar construct has a corresponding parse_* method. The parser reads
/// from a Lexeme_stream_reader, which provides arbitrary lookahead via peek()
/// and consumes tokens via read(). expect() is a helper that reads and asserts
/// a specific token, throwing on mismatch.
///
/// See grammar.md for the full grammar.

#include <format>
#include <limits>
#include <stdexcept>
#include <string>

#include <ast/operator.h>

#include "parsing/parser.h"

namespace benson
{

  Parser::Parser(
    Lexeme_stream_reader *reader,
    Spelling_table const *spellings
  ) noexcept
      : _reader{reader}, _spellings{spellings}
  {
  }

  ast::Translation_unit Parser::parse_translation_unit()
  {
    auto unit = ast::Translation_unit{};
    while (_reader->peek().token != Token::eof)
    {
      unit.statements.push_back(parse_statement());
    }
    return unit;
  }

  ast::Statement Parser::parse_statement()
  {
    auto const &next = _reader->peek();
    if (next.token == Token::kw_let)
    {
      return ast::Statement{parse_let_statement()};
    }
    if (next.token == Token::kw_return)
    {
      return ast::Statement{parse_return_statement()};
    }
    if (next.token == Token::kw_while)
    {
      return ast::Statement{parse_while_statement()};
    }
    return ast::Statement{parse_expression_statement()};
  }

  ast::Let_statement Parser::parse_let_statement()
  {
    auto stmt = ast::Let_statement{};
    stmt.kw_let = expect(Token::kw_let);
    if (_reader->peek().token == Token::kw_mut)
    {
      stmt.kw_mut = expect(Token::kw_mut);
    }
    stmt.name = expect(Token::identifier);
    stmt.eq = expect(Token::eq);
    stmt.initializer = std::move(*parse_expression());
    stmt.semicolon = expect(Token::semicolon);
    return stmt;
  }

  ast::Return_statement Parser::parse_return_statement()
  {
    auto stmt = ast::Return_statement{};
    stmt.kw_return = expect(Token::kw_return);
    stmt.value = std::move(*parse_expression());
    stmt.semicolon = expect(Token::semicolon);
    return stmt;
  }

  ast::Expression_statement Parser::parse_expression_statement()
  {
    auto stmt = ast::Expression_statement{};
    stmt.expression = std::move(*parse_expression());
    stmt.semicolon = expect(Token::semicolon);
    return stmt;
  }

  ast::Block_expression Parser::parse_block_expression()
  {
    auto block = ast::Block_expression{};
    block.lbrace = expect(Token::lbrace);
    while (_reader->peek().token != Token::rbrace)
    {
      auto const &next = _reader->peek();
      if (next.token == Token::kw_let)
      {
        block.statements.push_back(ast::Statement{parse_let_statement()});
      }
      else if (next.token == Token::kw_return)
      {
        block.statements.push_back(ast::Statement{parse_return_statement()});
      }
      else if (next.token == Token::kw_while)
      {
        block.statements.push_back(ast::Statement{parse_while_statement()});
      }
      else
      {
        auto expr = parse_expression();
        if (_reader->peek().token == Token::semicolon)
        {
          auto stmt = ast::Expression_statement{};
          stmt.expression = std::move(*expr);
          stmt.semicolon = expect(Token::semicolon);
          block.statements.push_back(ast::Statement{std::move(stmt)});
        }
        else
        {
          block.tail = std::move(expr);
          break;
        }
      }
    }
    block.rbrace = expect(Token::rbrace);
    return block;
  }

  std::unique_ptr<ast::Expression> Parser::parse_expression()
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
  /// 1. Add a token to `Token` (token.h) and lex it in
  /// lexeme_stream.cpp.
  /// 2. Add a variant to `ast::Operator` (operator.h).
  /// 3. Assign it a precedence in `get_operator_precedence` (operator.cpp).
  /// 4. Map the token to the operator in the appropriate get_*_operator
  /// function
  ///    (operator.cpp): `get_prefix_operator`, `get_postfix_operator`, or
  ///    `get_binary_operator`. No changes to the parser itself are needed.
  std::unique_ptr<ast::Expression>
  Parser::parse_expression(int current_precedence)
  {
    auto expr = [&]() -> std::unique_ptr<ast::Expression>
    {
      if (auto const prefix_op =
            ast::get_prefix_operator(_reader->peek().token);
          prefix_op &&
          ast::get_operator_precedence(*prefix_op) <= current_precedence)
      {
        auto prefix = ast::Prefix_expression{};
        prefix.op = _reader->read();
        prefix.operand =
          parse_expression(ast::get_operator_precedence(*prefix_op));
        return std::make_unique<ast::Expression>(std::move(prefix));
      }
      if (_reader->peek().token == Token::lbracket && 1 <= current_precedence)
      {
        auto prefix = ast::Prefix_bracket_expression{};
        prefix.lbracket = expect(Token::lbracket);
        if (_reader->peek().token != Token::rbracket)
        {
          prefix.size = parse_expression();
        }
        prefix.rbracket = expect(Token::rbracket);
        prefix.operand = parse_expression(1);
        return std::make_unique<ast::Expression>(std::move(prefix));
      }
      auto primary = parse_primary_expression();
      for (;;)
      {
        if (auto const postfix_op =
              ast::get_postfix_operator(_reader->peek().token);
            postfix_op &&
            ast::get_operator_precedence(*postfix_op) <= current_precedence)
        {
          if (*postfix_op == ast::Operator::call)
          {
            auto call = ast::Call_expression{};
            call.callee = std::move(primary);
            call.lparen = expect(Token::lparen);
            for (;;)
            {
              if (_reader->peek().token == Token::rparen)
              {
                break;
              }
              call.arguments.push_back(std::move(*parse_expression()));
              if (_reader->peek().token != Token::comma)
              {
                break;
              }
              call.argument_commas.push_back(expect(Token::comma));
            }
            call.rparen = expect(Token::rparen);
            primary = std::make_unique<ast::Expression>(std::move(call));
          }
          else if (*postfix_op == ast::Operator::index)
          {
            auto idx = ast::Index_expression{};
            idx.operand = std::move(primary);
            idx.lbracket = expect(Token::lbracket);
            idx.index = parse_expression();
            idx.rbracket = expect(Token::rbracket);
            primary = std::make_unique<ast::Expression>(std::move(idx));
          }
          else
          {
            auto postfix = ast::Postfix_expression{};
            postfix.operand = std::move(primary);
            postfix.op = _reader->read();
            primary = std::make_unique<ast::Expression>(std::move(postfix));
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
      if (auto const bin_op = ast::get_binary_operator(_reader->peek().token);
          bin_op && ast::get_operator_precedence(*bin_op) <= current_precedence)
      {
        auto const op_prec = ast::get_operator_precedence(*bin_op);
        auto const rhs_prec = ast::get_precedence_associativity(op_prec) ==
                                  ast::Operator_associativity::right
                                ? op_prec
                                : op_prec - 1;
        auto binary = ast::Binary_expression{};
        binary.left = std::move(expr);
        binary.op = _reader->read();
        binary.right = parse_expression(rhs_prec);
        expr = std::make_unique<ast::Expression>(std::move(binary));
      }
      else
      {
        break;
      }
    }
    return expr;
  }

  std::unique_ptr<ast::Expression> Parser::parse_primary_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == Token::int_literal)
    {
      return std::make_unique<ast::Expression>(parse_int_literal_expression());
    }
    if (next.token == Token::float_literal)
    {
      return std::make_unique<ast::Expression>(
        parse_float_literal_expression()
      );
    }
    if (next.token == Token::identifier)
    {
      return std::make_unique<ast::Expression>(parse_identifier_expression());
    }
    if (next.token == Token::kw_recurse)
    {
      return std::make_unique<ast::Expression>(
        ast::Recurse_expression{.kw_recurse = _reader->read()}
      );
    }
    if (next.token == Token::kw_if)
    {
      return std::make_unique<ast::Expression>(parse_if_expression());
    }
    if (next.token == Token::lbrace)
    {
      return std::make_unique<ast::Expression>(parse_block_expression());
    }
    if (next.token == Token::lparen)
    {
      return std::make_unique<ast::Expression>(parse_paren_expression());
    }
    if (next.token == Token::kw_fn)
    {
      return std::make_unique<ast::Expression>(parse_fn_expression());
    }
    throw std::runtime_error{
      std::format(
        "unexpected token '{}' at {}:{}",
        _spellings->lookup(next.spelling),
        next.span.start.line,
        next.span.start.column
      )
    };
  }

  ast::Fn_expression Parser::parse_fn_expression()
  {
    auto fn = ast::Fn_expression{};
    fn.kw_fn = expect(Token::kw_fn);
    if (_reader->peek().token == Token::identifier)
    {
      fn.name = expect(Token::identifier);
    }
    fn.lparen = expect(Token::lparen);
    for (;;)
    {
      if (_reader->peek().token == Token::rparen)
      {
        break;
      }
      auto param = ast::Fn_expression::Parameter_declaration{};
      if (_reader->peek().token == Token::kw_mut)
      {
        param.kw_mut = expect(Token::kw_mut);
      }
      param.name = expect(Token::identifier);
      param.colon = expect(Token::colon);
      param.type = parse_expression();
      fn.parameters.push_back(std::move(param));
      if (_reader->peek().token != Token::comma)
      {
        break;
      }
      fn.parameter_commas.push_back(expect(Token::comma));
    }
    fn.rparen = expect(Token::rparen);
    fn.return_type_specifier = parse_return_type_specifier();
    fn.arrow = expect(Token::fat_arrow);
    fn.body = parse_expression();
    return fn;
  }

  ast::Fn_expression::Return_type_specifier
  Parser::parse_return_type_specifier()
  {
    auto spec = ast::Fn_expression::Return_type_specifier{};
    spec.colon = expect(Token::colon);
    spec.type = parse_expression();
    return spec;
  }

  ast::Int_literal_expression Parser::parse_int_literal_expression()
  {
    return ast::Int_literal_expression{.literal = expect(Token::int_literal)};
  }

  ast::Float_literal_expression Parser::parse_float_literal_expression()
  {
    return ast::Float_literal_expression{
      .literal = expect(Token::float_literal)
    };
  }

  ast::Identifier_expression Parser::parse_identifier_expression()
  {
    return ast::Identifier_expression{.identifier = expect(Token::identifier)};
  }

  ast::Paren_expression Parser::parse_paren_expression()
  {
    auto expr = ast::Paren_expression{};
    expr.lparen = expect(Token::lparen);
    expr.inner = parse_expression();
    expr.rparen = expect(Token::rparen);
    return expr;
  }

  ast::If_expression Parser::parse_if_expression()
  {
    auto expr = ast::If_expression{};
    expr.kw_if = expect(Token::kw_if);
    expr.condition = parse_expression();
    expr.then_block = parse_block_expression();
    while (_reader->peek().token == Token::kw_else)
    {
      auto kw_else = expect(Token::kw_else);
      if (_reader->peek().token == Token::kw_if)
      {
        auto part = ast::If_expression::Else_if_part{};
        part.kw_else = kw_else;
        part.kw_if = expect(Token::kw_if);
        part.condition = parse_expression();
        part.body = parse_block_expression();
        expr.else_if_parts.push_back(std::move(part));
      }
      else
      {
        auto else_part = ast::If_expression::Else_part{};
        else_part.kw_else = kw_else;
        else_part.body = parse_block_expression();
        expr.else_part = std::move(else_part);
        break;
      }
    }
    return expr;
  }

  ast::While_statement Parser::parse_while_statement()
  {
    auto stmt = ast::While_statement{};
    stmt.kw_while = expect(Token::kw_while);
    stmt.condition = parse_expression();
    stmt.body = parse_block_expression();
    return stmt;
  }

  Lexeme Parser::expect(Token token)
  {
    auto lexeme = _reader->read();
    if (lexeme.token != token)
    {
      throw std::runtime_error{
        std::format(
          "unexpected token '{}' at {}:{}",
          _spellings->lookup(lexeme.spelling),
          lexeme.span.start.line,
          lexeme.span.start.column
        )
      };
    }
    return lexeme;
  }

} // namespace benson
