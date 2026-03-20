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
  struct Block_expression;

  // type expressions

  struct Identifier_type_expression
  {
    basedlex::Lexeme identifier;
  };

  struct Array_type_expression
  {
    std::unique_ptr<Type_expression> element_type;
    basedlex::Lexeme lbracket;
    std::unique_ptr<Expression> size; // nullptr for unsized arrays (e.g. i32[])
    basedlex::Lexeme rbracket;
  };

  struct Pointer_type_expression
  {
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
      std::optional<basedlex::Lexeme> kw_mut;
      basedlex::Lexeme name;
      basedlex::Lexeme colon;
      Type_expression type_expression;
    };

    struct Return_type_specifier
    {
      basedlex::Lexeme colon;
      Type_expression type_expression;
    };

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
    basedlex::Lexeme lparen;
    std::unique_ptr<Expression> inner;
    basedlex::Lexeme rparen;
  };

  struct Unary_expression
  {
    basedlex::Lexeme op;
    std::unique_ptr<Expression> operand;
  };

  struct Binary_expression
  {
    std::unique_ptr<Expression> left;
    basedlex::Lexeme op;
    std::unique_ptr<Expression> right;
  };

  struct Call_expression
  {
    std::unique_ptr<Expression> callee;
    basedlex::Lexeme lparen;
    std::vector<Expression> arguments;
    std::vector<basedlex::Lexeme> argument_commas;
    basedlex::Lexeme rparen;
  };

  struct Index_expression
  {
    std::unique_ptr<Expression> operand;
    basedlex::Lexeme lbracket;
    std::unique_ptr<Expression> index;
    basedlex::Lexeme rbracket;
  };

  struct Block_expression
  {
    basedlex::Lexeme lbrace;
    std::vector<Statement> statements;
    std::unique_ptr<Expression> tail;
    basedlex::Lexeme rbrace;
  };

  struct If_expression
  {
    struct Else_if_part
    {
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

    basedlex::Lexeme kw_if;
    std::unique_ptr<Expression> condition;
    Block_expression then_block;
    std::vector<Else_if_part> else_if_parts;
    std::optional<Else_part> else_part;
  };

  struct Constructor_expression
  {
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
      Block_expression,
      If_expression,
      Constructor_expression
    >
      value;
  };

  // statements

  struct While_statement
  {
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
      While_statement,
      Return_statement,
      Expression_statement,
      Function_definition
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
