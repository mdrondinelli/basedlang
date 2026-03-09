#ifndef BASEDPARSE_AST_H
#define BASEDPARSE_AST_H

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "basedlex/lexeme.h"

namespace basedparse
{

  struct Type_expression;
  struct Expression;
  struct Statement;
  struct Block_statement;

  // type expressions

  struct Identifier_type_expression
  {
    basedlex::Lexeme identifier;
  };

  struct Array_type_expression
  {
    ~Array_type_expression();

    Array_type_expression() = default;

    Array_type_expression(Array_type_expression &&) noexcept = default;

    Array_type_expression &
    operator=(Array_type_expression &&) noexcept = default;

    std::unique_ptr<Type_expression> element_type;
    basedlex::Lexeme lbracket;
    std::unique_ptr<Expression> size; // nullptr for unsized arrays (e.g. i32[])
    basedlex::Lexeme rbracket;
  };

  struct Pointer_type_expression
  {
    ~Pointer_type_expression();

    Pointer_type_expression() = default;

    Pointer_type_expression(Pointer_type_expression &&) noexcept = default;

    Pointer_type_expression &
    operator=(Pointer_type_expression &&) noexcept = default;

    std::unique_ptr<Type_expression> pointee_type;
    std::optional<basedlex::Lexeme> kw_mut;
    basedlex::Lexeme star;
  };

  struct Type_expression
  {
    std::variant<
      Identifier_type_expression,
      Array_type_expression,
      Pointer_type_expression
    >
      value;
  };

  // expressions

  struct Int_literal_expression
  {
    basedlex::Lexeme literal;
  };

  struct Identifier_expression
  {
    basedlex::Lexeme identifier;
  };

  struct Fn_expression
  {
    struct Parameter_declaration
    {
      basedlex::Lexeme name;
      basedlex::Lexeme colon;
      Type_expression type_expression;
    };

    struct Return_type_specifier
    {
      basedlex::Lexeme arrow;
      Type_expression type_expression;
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
    std::unique_ptr<Block_statement> body;
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

  struct Unary_expression
  {
    ~Unary_expression();

    Unary_expression() = default;

    Unary_expression(Unary_expression &&) noexcept = default;

    Unary_expression &operator=(Unary_expression &&) noexcept = default;

    basedlex::Lexeme op;
    std::unique_ptr<Expression> operand;
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

  struct Constructor_expression
  {
    ~Constructor_expression();

    Constructor_expression() = default;

    Constructor_expression(Constructor_expression &&) noexcept = default;

    Constructor_expression &
    operator=(Constructor_expression &&) noexcept = default;

    basedlex::Lexeme kw_new;
    Type_expression type;
    basedlex::Lexeme lbrace;
    std::vector<Expression> arguments;
    std::vector<basedlex::Lexeme> argument_commas;
    basedlex::Lexeme rbrace;
  };

  struct Expression
  {
    std::variant<
      Int_literal_expression,
      Identifier_expression,
      Fn_expression,
      Paren_expression,
      Unary_expression,
      Binary_expression,
      Call_expression,
      Index_expression,
      Constructor_expression
    >
      value;
  };

  // statements

  struct Let_statement
  {
    basedlex::Lexeme kw_let;
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

  struct Block_statement
  {
    ~Block_statement();

    Block_statement() = default;

    Block_statement(Block_statement &&) noexcept = default;

    Block_statement &operator=(Block_statement &&) noexcept = default;

    basedlex::Lexeme lbrace;
    std::vector<Statement> statements;
    basedlex::Lexeme rbrace;
  };

  struct Function_definition
  {
    basedlex::Lexeme kw_let;
    basedlex::Lexeme name;
    basedlex::Lexeme eq;
    Fn_expression function;
    basedlex::Lexeme semicolon;
  };

  struct Statement
  {
    std::variant<
      Let_statement,
      Return_statement,
      Expression_statement,
      Block_statement,
      Function_definition
    >
      value;
  };

  // translation unit

  struct Translation_unit
  {
    std::vector<Statement> statements;
  };

} // namespace basedparse

#endif // BASEDPARSE_AST_H
