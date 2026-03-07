#include <stdexcept>
#include <string>

#include "basedparse/parser.h"

namespace basedparse
{

  Parser::Parser(basedlex::Lexeme_stream_reader *reader) noexcept
      : _reader{reader}
  {
  }

  Translation_unit Parser::parse_translation_unit()
  {
    auto statements = std::vector<std::unique_ptr<Statement>>{};
    while (_reader->peek().token != basedlex::Token::eof)
    {
      statements.push_back(parse_statement());
    }
    return Translation_unit{std::move(statements)};
  }

  std::unique_ptr<Statement> Parser::parse_statement()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::kw_let)
    {
      // Peek ahead: let <name> = fn means Function_definition
      if (_reader->peek(3).token == basedlex::Token::kw_fn)
      {
        auto kw_let = _reader->read();
        auto name = expect(basedlex::Token::identifier);
        auto eq = expect(basedlex::Token::eq);
        auto function = parse_fn_expression();
        auto stmt = std::make_unique<Function_definition>();
        stmt->kw_let = std::move(kw_let);
        stmt->name = std::move(name);
        stmt->eq = std::move(eq);
        stmt->function = std::move(function);
        return stmt;
      }
      auto kw_let = _reader->read();
      auto name = expect(basedlex::Token::identifier);
      auto eq = expect(basedlex::Token::eq);
      auto initializer = parse_expression();
      auto semicolon = expect(basedlex::Token::semicolon);
      auto stmt = std::make_unique<Let_statement>();
      stmt->kw_let = std::move(kw_let);
      stmt->name = std::move(name);
      stmt->eq = std::move(eq);
      stmt->initializer = std::move(initializer);
      stmt->semicolon = std::move(semicolon);
      return stmt;
    }
    if (next.token == basedlex::Token::kw_return)
    {
      auto kw_return = _reader->read();
      auto value = parse_expression();
      auto semicolon = expect(basedlex::Token::semicolon);
      auto stmt = std::make_unique<Return_statement>();
      stmt->kw_return = std::move(kw_return);
      stmt->value = std::move(value);
      stmt->semicolon = std::move(semicolon);
      return stmt;
    }
    auto expression = parse_expression();
    auto semicolon = expect(basedlex::Token::semicolon);
    auto stmt = std::make_unique<Expression_statement>();
    stmt->expression = std::move(expression);
    stmt->semicolon = std::move(semicolon);
    return stmt;
  }

  std::unique_ptr<Expression> Parser::parse_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::int_literal)
    {
      auto expr = std::make_unique<Int_literal_expression>();
      expr->literal = _reader->read();
      return expr;
    }
    if (next.token == basedlex::Token::identifier)
    {
      auto expr = std::make_unique<Identifier_expression>();
      expr->identifier = _reader->read();
      return expr;
    }
    if (next.token == basedlex::Token::kw_fn)
    {
      auto fn = parse_fn_expression();
      return std::make_unique<Fn_expression>(std::move(fn));
    }
    throw std::runtime_error{
      "unexpected token '" + next.text + "' at " + std::to_string(next.line) +
      ":" + std::to_string(next.column)
    };
  }

  std::unique_ptr<Type_expression> Parser::parse_type_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::identifier)
    {
      auto expr = std::make_unique<Identifier_type_expression>();
      expr->identifier = _reader->read();
      return expr;
    }
    throw std::runtime_error{
      "expected type, got '" + next.text + "' at " + std::to_string(next.line) +
      ":" + std::to_string(next.column)
    };
  }

  Fn_expression Parser::parse_fn_expression()
  {
    auto fn = Fn_expression{};
    fn.kw_fn = expect(basedlex::Token::kw_fn);
    fn.lparen = expect(basedlex::Token::lparen);
    fn.rparen = expect(basedlex::Token::rparen);
    fn.arrow = expect(basedlex::Token::arrow);
    fn.return_type = parse_type_expression();
    fn.body = parse_block_statement();
    return fn;
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
