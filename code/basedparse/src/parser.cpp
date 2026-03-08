#include <stdexcept>
#include <string>

#include "basedparse/parser.h"

namespace basedparse
{

  Parser::Parser(basedlex::Lexeme_stream_reader *reader) noexcept
      : _reader{reader}
  {
  }

  std::unique_ptr<Translation_unit> Parser::parse_translation_unit()
  {
    auto unit = std::make_unique<Translation_unit>();
    while (_reader->peek().token != basedlex::Token::eof)
    {
      unit->statements.push_back(parse_function_definition());
    }
    return unit;
  }

  std::unique_ptr<Function_definition> Parser::parse_function_definition()
  {
    auto stmt = std::make_unique<Function_definition>();
    stmt->kw_let = expect(basedlex::Token::kw_let);
    stmt->name = expect(basedlex::Token::identifier);
    stmt->eq = expect(basedlex::Token::eq);
    stmt->function = std::move(*parse_fn_expression());
    return stmt;
  }

  std::unique_ptr<Statement> Parser::parse_statement()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::kw_let)
    {
      return parse_let_statement();
    }
    if (next.token == basedlex::Token::kw_return)
    {
      return parse_return_statement();
    }
    return parse_expression_statement();
  }

  std::unique_ptr<Let_statement> Parser::parse_let_statement()
  {
    auto stmt = std::make_unique<Let_statement>();
    stmt->kw_let = expect(basedlex::Token::kw_let);
    stmt->name = expect(basedlex::Token::identifier);
    stmt->eq = expect(basedlex::Token::eq);
    stmt->initializer = parse_expression();
    stmt->semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  std::unique_ptr<Return_statement> Parser::parse_return_statement()
  {
    auto stmt = std::make_unique<Return_statement>();
    stmt->kw_return = expect(basedlex::Token::kw_return);
    stmt->value = parse_expression();
    stmt->semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  std::unique_ptr<Expression_statement> Parser::parse_expression_statement()
  {
    auto stmt = std::make_unique<Expression_statement>();
    stmt->expression = parse_expression();
    stmt->semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  std::unique_ptr<Block_statement> Parser::parse_block_statement()
  {
    auto block = std::make_unique<Block_statement>();
    block->lbrace = expect(basedlex::Token::lbrace);
    while (_reader->peek().token != basedlex::Token::rbrace)
    {
      block->statements.push_back(parse_statement());
    }
    block->rbrace = expect(basedlex::Token::rbrace);
    return block;
  }

  std::unique_ptr<Expression> Parser::parse_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::int_literal)
    {
      return parse_int_literal_expression();
    }
    if (next.token == basedlex::Token::identifier)
    {
      return parse_identifier_expression();
    }
    if (next.token == basedlex::Token::lparen)
    {
      return parse_paren_expression();
    }
    if (next.token == basedlex::Token::kw_fn)
    {
      return parse_fn_expression();
    }
    throw std::runtime_error{
      "unexpected token '" + next.text + "' at " + std::to_string(next.line) +
      ":" + std::to_string(next.column)
    };
  }

  std::unique_ptr<Fn_expression> Parser::parse_fn_expression()
  {
    auto fn = std::make_unique<Fn_expression>();
    fn->kw_fn = expect(basedlex::Token::kw_fn);
    fn->lparen = expect(basedlex::Token::lparen);
    fn->rparen = expect(basedlex::Token::rparen);
    fn->arrow = expect(basedlex::Token::arrow);
    fn->return_type = parse_type_expression();
    fn->body = parse_block_statement();
    return fn;
  }

  std::unique_ptr<Int_literal_expression> Parser::parse_int_literal_expression()
  {
    auto expr = std::make_unique<Int_literal_expression>();
    expr->literal = expect(basedlex::Token::int_literal);
    return expr;
  }

  std::unique_ptr<Identifier_expression> Parser::parse_identifier_expression()
  {
    auto expr = std::make_unique<Identifier_expression>();
    expr->identifier = expect(basedlex::Token::identifier);
    return expr;
  }

  std::unique_ptr<Paren_expression> Parser::parse_paren_expression()
  {
    auto expr = std::make_unique<Paren_expression>();
    expr->lparen = expect(basedlex::Token::lparen);
    expr->inner = parse_expression();
    expr->rparen = expect(basedlex::Token::rparen);
    return expr;
  }

  std::unique_ptr<Type_expression> Parser::parse_type_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::identifier)
    {
      return parse_identifier_type_expression();
    }
    throw std::runtime_error{
      "expected type, got '" + next.text + "' at " + std::to_string(next.line) +
      ":" + std::to_string(next.column)
    };
  }

  std::unique_ptr<Identifier_type_expression>
  Parser::parse_identifier_type_expression()
  {
    auto expr = std::make_unique<Identifier_type_expression>();
    expr->identifier = expect(basedlex::Token::identifier);
    return expr;
  }

  basedlex::Lexeme Parser::expect(basedlex::Token token)
  {
    auto lexeme = _reader->read();
    if (lexeme.token != token)
    {
      throw std::runtime_error{
        "unexpected token '" + lexeme.text + "' at " +
        std::to_string(lexeme.line) + ":" + std::to_string(lexeme.column)
      };
    }
    return lexeme;
  }

} // namespace basedparse
