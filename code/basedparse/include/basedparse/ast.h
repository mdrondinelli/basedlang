#ifndef BASEDPARSE_AST_H
#define BASEDPARSE_AST_H

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "basedlex/lexeme.h"

namespace basedparse
{

  struct Expression;
  struct Statement;
  struct Block_expression;

  // expressions

  struct Int_literal_expression
  {
    basedlex::Lexeme literal;
  };

  struct Identifier_expression
  {
    basedlex::Lexeme identifier;
  };

  struct Recurse_expression
  {
    basedlex::Lexeme kw_recurse;
  };

  struct Fn_expression
  {
    struct Parameter_declaration
    {
      std::optional<basedlex::Lexeme> kw_mut;
      basedlex::Lexeme name;
      basedlex::Lexeme colon;
      std::unique_ptr<Expression> type;
    };

    struct Return_type_specifier
    {
      basedlex::Lexeme colon;
      std::unique_ptr<Expression> type;
    };

    ~Fn_expression();

    Fn_expression() = default;

    Fn_expression(Fn_expression &&) noexcept = default;

    Fn_expression &operator=(Fn_expression &&) noexcept = default;

    basedlex::Lexeme kw_fn;
    basedlex::Lexeme lparen;
    std::vector<Parameter_declaration> parameters;
    std::vector<basedlex::Lexeme> parameter_commas;
    basedlex::Lexeme rparen;
    std::optional<Return_type_specifier> return_type_specifier;
    basedlex::Lexeme arrow;
    std::unique_ptr<Expression> body;
  };

  struct Paren_expression
  {
    ~Paren_expression();

    Paren_expression() = default;

    Paren_expression(Paren_expression &&) noexcept = default;

    Paren_expression &operator=(Paren_expression &&) noexcept = default;

    basedlex::Lexeme lparen;
    std::unique_ptr<Expression> inner;
    basedlex::Lexeme rparen;
  };

  struct Prefix_expression
  {
    ~Prefix_expression();

    Prefix_expression() = default;

    Prefix_expression(Prefix_expression &&) noexcept = default;

    Prefix_expression &operator=(Prefix_expression &&) noexcept = default;

    basedlex::Lexeme op;
    std::unique_ptr<Expression> operand;
  };

  struct Postfix_expression
  {
    ~Postfix_expression();

    Postfix_expression() = default;

    Postfix_expression(Postfix_expression &&) noexcept = default;

    Postfix_expression &operator=(Postfix_expression &&) noexcept = default;

    std::unique_ptr<Expression> operand;
    basedlex::Lexeme op;
  };

  struct Binary_expression
  {
    ~Binary_expression();

    Binary_expression() = default;

    Binary_expression(Binary_expression &&) noexcept = default;

    Binary_expression &operator=(Binary_expression &&) noexcept = default;

    std::unique_ptr<Expression> left;
    basedlex::Lexeme op;
    std::unique_ptr<Expression> right;
  };

  struct Call_expression
  {
    ~Call_expression();

    Call_expression() = default;

    Call_expression(Call_expression &&) noexcept = default;

    Call_expression &operator=(Call_expression &&) noexcept = default;

    std::unique_ptr<Expression> callee;
    basedlex::Lexeme lparen;
    std::vector<Expression> arguments;
    std::vector<basedlex::Lexeme> argument_commas;
    basedlex::Lexeme rparen;
  };

  struct Index_expression
  {
    ~Index_expression();

    Index_expression() = default;

    Index_expression(Index_expression &&) noexcept = default;

    Index_expression &operator=(Index_expression &&) noexcept = default;

    std::unique_ptr<Expression> operand;
    basedlex::Lexeme lbracket;
    std::unique_ptr<Expression> index;
    basedlex::Lexeme rbracket;
  };

  struct Prefix_bracket_expression
  {
    ~Prefix_bracket_expression();

    Prefix_bracket_expression() = default;

    Prefix_bracket_expression(Prefix_bracket_expression &&) noexcept = default;

    Prefix_bracket_expression &
    operator=(Prefix_bracket_expression &&) noexcept = default;

    basedlex::Lexeme lbracket;
    std::unique_ptr<Expression> size;
    basedlex::Lexeme rbracket;
    std::unique_ptr<Expression> operand;
  };

  struct Block_expression
  {
    ~Block_expression();

    Block_expression() = default;

    Block_expression(Block_expression &&) noexcept = default;

    Block_expression &operator=(Block_expression &&) noexcept = default;

    basedlex::Lexeme lbrace;
    std::vector<Statement> statements;
    std::unique_ptr<Expression> tail;
    basedlex::Lexeme rbrace;
  };

  struct If_expression
  {
    struct Else_if_part
    {
      ~Else_if_part();

      Else_if_part() = default;

      Else_if_part(Else_if_part &&) noexcept = default;

      Else_if_part &operator=(Else_if_part &&) noexcept = default;

      basedlex::Lexeme kw_else;
      basedlex::Lexeme kw_if;
      std::unique_ptr<Expression> condition;
      Block_expression body;
    };

    struct Else_part
    {
      basedlex::Lexeme kw_else;
      Block_expression body;
    };

    ~If_expression();

    If_expression() = default;

    If_expression(If_expression &&) noexcept = default;

    If_expression &operator=(If_expression &&) noexcept = default;

    basedlex::Lexeme kw_if;
    std::unique_ptr<Expression> condition;
    Block_expression then_block;
    std::vector<Else_if_part> else_if_parts;
    std::optional<Else_part> else_part;
  };

  struct Expression
  {
    std::variant<
      Int_literal_expression,
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

    basedlex::Lexeme kw_while;
    std::unique_ptr<Expression> condition;
    Block_expression body;
  };

  struct Let_statement
  {
    basedlex::Lexeme kw_let;
    std::optional<basedlex::Lexeme> kw_mut;
    basedlex::Lexeme name;
    basedlex::Lexeme eq;
    Expression initializer;
    basedlex::Lexeme semicolon;
  };

  struct Return_statement
  {
    basedlex::Lexeme kw_return;
    Expression value;
    basedlex::Lexeme semicolon;
  };

  struct Expression_statement
  {
    Expression expression;
    basedlex::Lexeme semicolon;
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

} // namespace basedparse

#endif // BASEDPARSE_AST_H
