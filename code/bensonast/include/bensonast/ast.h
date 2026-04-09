#ifndef BASEDAST_AST_H
#define BASEDAST_AST_H

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "bensonlex/lexeme.h"

namespace benson::ast
{

  struct Expression;
  struct Statement;
  struct Block_expression;

  // expressions

  struct Int_literal_expression
  {
    bensonlex::Lexeme literal;
  };

  struct Float_literal_expression
  {
    bensonlex::Lexeme literal;
  };

  struct Identifier_expression
  {
    bensonlex::Lexeme identifier;
  };

  struct Recurse_expression
  {
    bensonlex::Lexeme kw_recurse;
  };

  struct Fn_expression
  {
    struct Parameter_declaration
    {
      std::optional<bensonlex::Lexeme> kw_mut;
      bensonlex::Lexeme name;
      bensonlex::Lexeme colon;
      std::unique_ptr<Expression> type;
    };

    struct Return_type_specifier
    {
      bensonlex::Lexeme colon;
      std::unique_ptr<Expression> type;
    };

    ~Fn_expression();

    Fn_expression() = default;

    Fn_expression(Fn_expression &&) noexcept = default;

    Fn_expression &operator=(Fn_expression &&) noexcept = default;

    bensonlex::Lexeme kw_fn;
    bensonlex::Lexeme lparen;
    std::vector<Parameter_declaration> parameters;
    std::vector<bensonlex::Lexeme> parameter_commas;
    bensonlex::Lexeme rparen;
    Return_type_specifier return_type_specifier;
    bensonlex::Lexeme arrow;
    std::unique_ptr<Expression> body;
  };

  struct Paren_expression
  {
    ~Paren_expression();

    Paren_expression() = default;

    Paren_expression(Paren_expression &&) noexcept = default;

    Paren_expression &operator=(Paren_expression &&) noexcept = default;

    bensonlex::Lexeme lparen;
    std::unique_ptr<Expression> inner;
    bensonlex::Lexeme rparen;
  };

  struct Prefix_expression
  {
    ~Prefix_expression();

    Prefix_expression() = default;

    Prefix_expression(Prefix_expression &&) noexcept = default;

    Prefix_expression &operator=(Prefix_expression &&) noexcept = default;

    bensonlex::Lexeme op;
    std::unique_ptr<Expression> operand;
  };

  struct Postfix_expression
  {
    ~Postfix_expression();

    Postfix_expression() = default;

    Postfix_expression(Postfix_expression &&) noexcept = default;

    Postfix_expression &operator=(Postfix_expression &&) noexcept = default;

    std::unique_ptr<Expression> operand;
    bensonlex::Lexeme op;
  };

  struct Binary_expression
  {
    ~Binary_expression();

    Binary_expression() = default;

    Binary_expression(Binary_expression &&) noexcept = default;

    Binary_expression &operator=(Binary_expression &&) noexcept = default;

    std::unique_ptr<Expression> left;
    bensonlex::Lexeme op;
    std::unique_ptr<Expression> right;
  };

  struct Call_expression
  {
    ~Call_expression();

    Call_expression() = default;

    Call_expression(Call_expression &&) noexcept = default;

    Call_expression &operator=(Call_expression &&) noexcept = default;

    std::unique_ptr<Expression> callee;
    bensonlex::Lexeme lparen;
    std::vector<Expression> arguments;
    std::vector<bensonlex::Lexeme> argument_commas;
    bensonlex::Lexeme rparen;
  };

  struct Index_expression
  {
    ~Index_expression();

    Index_expression() = default;

    Index_expression(Index_expression &&) noexcept = default;

    Index_expression &operator=(Index_expression &&) noexcept = default;

    std::unique_ptr<Expression> operand;
    bensonlex::Lexeme lbracket;
    std::unique_ptr<Expression> index;
    bensonlex::Lexeme rbracket;
  };

  struct Prefix_bracket_expression
  {
    ~Prefix_bracket_expression();

    Prefix_bracket_expression() = default;

    Prefix_bracket_expression(Prefix_bracket_expression &&) noexcept = default;

    Prefix_bracket_expression &
    operator=(Prefix_bracket_expression &&) noexcept = default;

    bensonlex::Lexeme lbracket;
    std::unique_ptr<Expression> size;
    bensonlex::Lexeme rbracket;
    std::unique_ptr<Expression> operand;
  };

  struct Block_expression
  {
    ~Block_expression();

    Block_expression() = default;

    Block_expression(Block_expression &&) noexcept = default;

    Block_expression &operator=(Block_expression &&) noexcept = default;

    bensonlex::Lexeme lbrace;
    std::vector<Statement> statements;
    std::unique_ptr<Expression> tail;
    bensonlex::Lexeme rbrace;
  };

  struct If_expression
  {
    struct Else_if_part
    {
      ~Else_if_part();

      Else_if_part() = default;

      Else_if_part(Else_if_part &&) noexcept = default;

      Else_if_part &operator=(Else_if_part &&) noexcept = default;

      bensonlex::Lexeme kw_else;
      bensonlex::Lexeme kw_if;
      std::unique_ptr<Expression> condition;
      Block_expression body;
    };

    struct Else_part
    {
      bensonlex::Lexeme kw_else;
      Block_expression body;
    };

    ~If_expression();

    If_expression() = default;

    If_expression(If_expression &&) noexcept = default;

    If_expression &operator=(If_expression &&) noexcept = default;

    bensonlex::Lexeme kw_if;
    std::unique_ptr<Expression> condition;
    Block_expression then_block;
    std::vector<Else_if_part> else_if_parts;
    std::optional<Else_part> else_part;
  };

  struct Expression
  {
    std::variant<
      Int_literal_expression,
      Float_literal_expression,
      Identifier_expression,
      Recurse_expression,
      Fn_expression,
      Paren_expression,
      Prefix_expression,
      Postfix_expression,
      Binary_expression,
      Call_expression,
      Index_expression,
      Prefix_bracket_expression,
      Block_expression,
      If_expression
    >
      value;
  };

  // statements

  struct While_statement
  {
    ~While_statement();

    While_statement() = default;

    While_statement(While_statement &&) noexcept = default;

    While_statement &operator=(While_statement &&) noexcept = default;

    bensonlex::Lexeme kw_while;
    std::unique_ptr<Expression> condition;
    Block_expression body;
  };

  struct Let_statement
  {
    bensonlex::Lexeme kw_let;
    std::optional<bensonlex::Lexeme> kw_mut;
    bensonlex::Lexeme name;
    bensonlex::Lexeme eq;
    Expression initializer;
    bensonlex::Lexeme semicolon;
  };

  struct Return_statement
  {
    bensonlex::Lexeme kw_return;
    Expression value;
    bensonlex::Lexeme semicolon;
  };

  struct Expression_statement
  {
    Expression expression;
    bensonlex::Lexeme semicolon;
  };

  struct Statement
  {
    std::variant<
      Let_statement,
      While_statement,
      Return_statement,
      Expression_statement
    >
      value;
  };

  // translation unit

  struct Translation_unit
  {
    std::vector<Let_statement> let_statements;
  };

} // namespace benson::ast

#endif // BASEDAST_AST_H
