#include <limits>
#include <stdexcept>
#include <string>

#include "basedparse/operator.h"
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
      unit->statements.push_back(Statement{parse_function_definition()});
    }
    return unit;
  }

  Function_definition Parser::parse_function_definition()
  {
    auto stmt = Function_definition{};
    stmt.kw_let = expect(basedlex::Token::kw_let);
    stmt.name = expect(basedlex::Token::identifier);
    stmt.eq = expect(basedlex::Token::eq);
    stmt.function = parse_fn_expression();
    stmt.semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  Statement Parser::parse_statement()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::kw_let)
    {
      return Statement{parse_let_statement()};
    }
    if (next.token == basedlex::Token::kw_return)
    {
      return Statement{parse_return_statement()};
    }
    return Statement{parse_expression_statement()};
  }

  Let_statement Parser::parse_let_statement()
  {
    auto stmt = Let_statement{};
    stmt.kw_let = expect(basedlex::Token::kw_let);
    stmt.name = expect(basedlex::Token::identifier);
    stmt.eq = expect(basedlex::Token::eq);
    stmt.initializer = std::move(*parse_expression());
    stmt.semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  Return_statement Parser::parse_return_statement()
  {
    auto stmt = Return_statement{};
    stmt.kw_return = expect(basedlex::Token::kw_return);
    stmt.value = std::move(*parse_expression());
    stmt.semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  Expression_statement Parser::parse_expression_statement()
  {
    auto stmt = Expression_statement{};
    stmt.expression = std::move(*parse_expression());
    stmt.semicolon = expect(basedlex::Token::semicolon);
    return stmt;
  }

  Block_expression Parser::parse_block_expression()
  {
    auto block = Block_expression{};
    block.lbrace = expect(basedlex::Token::lbrace);
    while (_reader->peek().token != basedlex::Token::rbrace)
    {
      auto const &next = _reader->peek();
      if (next.token == basedlex::Token::kw_let)
      {
        block.statements.push_back(Statement{parse_let_statement()});
      }
      else if (next.token == basedlex::Token::kw_return)
      {
        block.statements.push_back(Statement{parse_return_statement()});
      }
      else
      {
        auto expr = parse_expression();
        if (_reader->peek().token == basedlex::Token::semicolon)
        {
          auto stmt = Expression_statement{};
          stmt.expression = std::move(*expr);
          stmt.semicolon = expect(basedlex::Token::semicolon);
          block.statements.push_back(Statement{std::move(stmt)});
        }
        else
        {
          block.tail = std::move(expr);
          break;
        }
      }
    }
    block.rbrace = expect(basedlex::Token::rbrace);
    return block;
  }

  std::unique_ptr<Expression> Parser::parse_expression()
  {
    return parse_expression(std::numeric_limits<int>::max());
  }

  std::unique_ptr<Expression> Parser::parse_expression(int current_precedence)
  {
    auto expr = [&]() -> std::unique_ptr<Expression>
    {
      if (auto const unary_op = get_unary_operator(_reader->peek().token);
          unary_op && get_operator_precedence(*unary_op) <= current_precedence)
      {
        auto unary = Unary_expression{};
        unary.op = _reader->read();
        unary.operand =
          parse_expression(get_operator_precedence(*unary_op) - 1);
        return std::make_unique<Expression>(std::move(unary));
      }
      auto primary = parse_primary_expression();
      for (;;)
      {
        if (auto const postfix_op = get_postfix_operator(_reader->peek().token);
            postfix_op &&
            get_operator_precedence(*postfix_op) <= current_precedence)
        {
          if (*postfix_op == Operator::call)
          {
            auto call = Call_expression{};
            call.callee = std::move(primary);
            call.lparen = expect(basedlex::Token::lparen);
            for (;;)
            {
              if (_reader->peek().token == basedlex::Token::rparen)
              {
                break;
              }
              call.arguments.push_back(std::move(*parse_expression()));
              if (_reader->peek().token != basedlex::Token::comma)
              {
                break;
              }
              call.argument_commas.push_back(expect(basedlex::Token::comma));
            }
            call.rparen = expect(basedlex::Token::rparen);
            primary = std::make_unique<Expression>(std::move(call));
          }
          else
          {
            auto idx = Index_expression{};
            idx.operand = std::move(primary);
            idx.lbracket = expect(basedlex::Token::lbracket);
            idx.index = parse_expression();
            idx.rbracket = expect(basedlex::Token::rbracket);
            primary = std::make_unique<Expression>(std::move(idx));
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
      if (auto const bin_op = get_binary_operator(_reader->peek().token);
          bin_op && get_operator_precedence(*bin_op) <= current_precedence)
      {
        auto const op_prec = get_operator_precedence(*bin_op);
        auto binary = Binary_expression{};
        binary.left = std::move(expr);
        binary.op = _reader->read();
        binary.right = parse_expression(op_prec - 1);
        expr = std::make_unique<Expression>(std::move(binary));
      }
      else
      {
        break;
      }
    }
    return expr;
  }

  std::unique_ptr<Expression> Parser::parse_primary_expression()
  {
    auto const &next = _reader->peek();
    if (next.token == basedlex::Token::int_literal)
    {
      return std::make_unique<Expression>(parse_int_literal_expression());
    }
    if (next.token == basedlex::Token::identifier)
    {
      return std::make_unique<Expression>(parse_identifier_expression());
    }
    if (next.token == basedlex::Token::kw_new)
    {
      return std::make_unique<Expression>(parse_constructor_expression());
    }
    if (next.token == basedlex::Token::kw_if)
    {
      return std::make_unique<Expression>(parse_if_expression());
    }
    if (next.token == basedlex::Token::lbrace)
    {
      return std::make_unique<Expression>(parse_block_expression());
    }
    if (next.token == basedlex::Token::lparen)
    {
      return std::make_unique<Expression>(parse_paren_expression());
    }
    if (next.token == basedlex::Token::kw_fn)
    {
      return std::make_unique<Expression>(parse_fn_expression());
    }
    throw std::runtime_error{
      "unexpected token '" + next.text + "' at " + std::to_string(next.line) +
      ":" + std::to_string(next.column)
    };
  }

  Fn_expression Parser::parse_fn_expression()
  {
    auto fn = Fn_expression{};
    fn.kw_fn = expect(basedlex::Token::kw_fn);
    fn.lparen = expect(basedlex::Token::lparen);
    for (;;)
    {
      if (_reader->peek().token == basedlex::Token::rparen)
      {
        break;
      }
      auto param = Fn_expression::Parameter_declaration{};
      if (_reader->peek().token == basedlex::Token::kw_mut)
      {
        param.kw_mut = expect(basedlex::Token::kw_mut);
      }
      param.name = expect(basedlex::Token::identifier);
      param.colon = expect(basedlex::Token::colon);
      param.type_expression = std::move(*parse_type_expression());
      fn.parameters.push_back(std::move(param));
      if (_reader->peek().token != basedlex::Token::comma)
      {
        break;
      }
      fn.parameter_commas.push_back(expect(basedlex::Token::comma));
    }
    fn.rparen = expect(basedlex::Token::rparen);
    if (_reader->peek().token == basedlex::Token::arrow)
    {
      fn.return_type_specifier = parse_return_type_specifier();
    }
    fn.body = std::make_unique<Block_expression>(parse_block_expression());
    return fn;
  }

  Fn_expression::Return_type_specifier Parser::parse_return_type_specifier()
  {
    auto spec = Fn_expression::Return_type_specifier{};
    spec.arrow = expect(basedlex::Token::arrow);
    spec.type_expression = std::move(*parse_type_expression());
    return spec;
  }

  Int_literal_expression Parser::parse_int_literal_expression()
  {
    return Int_literal_expression{
      .literal = expect(basedlex::Token::int_literal)
    };
  }

  Identifier_expression Parser::parse_identifier_expression()
  {
    return Identifier_expression{
      .identifier = expect(basedlex::Token::identifier)
    };
  }

  Paren_expression Parser::parse_paren_expression()
  {
    auto expr = Paren_expression{};
    expr.lparen = expect(basedlex::Token::lparen);
    expr.inner = parse_expression();
    expr.rparen = expect(basedlex::Token::rparen);
    return expr;
  }

  If_expression Parser::parse_if_expression()
  {
    auto expr = If_expression{};
    expr.kw_if = expect(basedlex::Token::kw_if);
    expr.condition = parse_expression();
    expr.then_block = parse_block_expression();
    if (_reader->peek().token == basedlex::Token::kw_else)
    {
      auto else_clause = If_expression::Else_clause{};
      else_clause.kw_else = expect(basedlex::Token::kw_else);
      if (_reader->peek().token == basedlex::Token::kw_if)
      {
        else_clause.body = std::make_unique<Expression>(parse_if_expression());
      }
      else
      {
        else_clause.body =
          std::make_unique<Expression>(parse_block_expression());
      }
      expr.else_clause = std::move(else_clause);
    }
    return expr;
  }

  Constructor_expression Parser::parse_constructor_expression()
  {
    auto ctor = Constructor_expression{};
    ctor.kw_new = expect(basedlex::Token::kw_new);
    ctor.type = std::move(*parse_type_expression());
    ctor.lbrace = expect(basedlex::Token::lbrace);
    for (;;)
    {
      if (_reader->peek().token == basedlex::Token::rbrace)
      {
        break;
      }
      ctor.arguments.push_back(std::move(*parse_expression()));
      if (_reader->peek().token != basedlex::Token::comma)
      {
        break;
      }
      ctor.argument_commas.push_back(expect(basedlex::Token::comma));
    }
    ctor.rbrace = expect(basedlex::Token::rbrace);
    return ctor;
  }

  std::unique_ptr<Type_expression> Parser::parse_type_expression()
  {
    auto const &next = _reader->peek();
    if (next.token != basedlex::Token::identifier)
    {
      throw std::runtime_error{
        "expected type, got '" + next.text + "' at " +
        std::to_string(next.line) + ":" + std::to_string(next.column)
      };
    }
    auto type = std::make_unique<Type_expression>(Identifier_type_expression{
      .identifier = expect(basedlex::Token::identifier)
    });
    for (;;)
    {
      if (_reader->peek().token == basedlex::Token::lbracket)
      {
        auto array = Array_type_expression{};
        array.element_type = std::move(type);
        array.lbracket = expect(basedlex::Token::lbracket);
        if (_reader->peek().token != basedlex::Token::rbracket)
        {
          array.size = parse_expression();
        }
        array.rbracket = expect(basedlex::Token::rbracket);
        type = std::make_unique<Type_expression>(std::move(array));
      }
      else if (_reader->peek().token == basedlex::Token::kw_mut &&
               _reader->peek(1).token == basedlex::Token::star)
      {
        auto ptr = Pointer_type_expression{};
        ptr.pointee_type = std::move(type);
        ptr.kw_mut = expect(basedlex::Token::kw_mut);
        ptr.star = expect(basedlex::Token::star);
        type = std::make_unique<Type_expression>(std::move(ptr));
      }
      else if (_reader->peek().token == basedlex::Token::star)
      {
        auto ptr = Pointer_type_expression{};
        ptr.pointee_type = std::move(type);
        ptr.star = expect(basedlex::Token::star);
        type = std::make_unique<Type_expression>(std::move(ptr));
      }
      else
      {
        break;
      }
    }
    return type;
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
