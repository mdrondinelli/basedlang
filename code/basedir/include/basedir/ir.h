#ifndef BASEDIR_IR_H
#define BASEDIR_IR_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "basedparse/operator.h"

namespace basedir
{

  struct Expression;
  struct Statement;
  struct Block_expression;

  // expressions

  struct Int_literal_expression
  {
    int value;
  };

  struct Local_expression
  {
    std::size_t index;
  };

  struct Global_expression
  {
    std::size_t index;
  };

  struct Unary_expression
  {
    ~Unary_expression();

    Unary_expression() = default;

    Unary_expression(Unary_expression &&) noexcept = default;

    Unary_expression &operator=(Unary_expression &&) noexcept = default;

    basedparse::Operator op;
    std::unique_ptr<Expression> operand;
  };

  struct Binary_expression
  {
    ~Binary_expression();

    Binary_expression() = default;

    Binary_expression(Binary_expression &&) noexcept = default;

    Binary_expression &operator=(Binary_expression &&) noexcept = default;

    basedparse::Operator op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
  };

  struct Assign_expression
  {
    ~Assign_expression();

    Assign_expression() = default;

    Assign_expression(Assign_expression &&) noexcept = default;

    Assign_expression &operator=(Assign_expression &&) noexcept = default;

    std::unique_ptr<Expression> target;
    std::unique_ptr<Expression> value;
  };

  struct Call_expression
  {
    ~Call_expression();

    Call_expression() = default;

    Call_expression(Call_expression &&) noexcept = default;

    Call_expression &operator=(Call_expression &&) noexcept = default;

    std::unique_ptr<Expression> callee;
    std::vector<Expression> arguments;
  };

  struct Block_expression
  {
    ~Block_expression();

    Block_expression() = default;

    Block_expression(Block_expression &&) noexcept = default;

    Block_expression &operator=(Block_expression &&) noexcept = default;

    std::vector<Statement> statements;
    std::unique_ptr<Expression> tail;
  };

  struct If_expression
  {
    ~If_expression();

    If_expression() = default;

    If_expression(If_expression &&) noexcept = default;

    If_expression &operator=(If_expression &&) noexcept = default;

    std::unique_ptr<Expression> condition;
    Block_expression then_block;
    std::unique_ptr<Expression> else_body;
  };

  struct Expression
  {
    std::variant<
      Int_literal_expression,
      Local_expression,
      Global_expression,
      Unary_expression,
      Binary_expression,
      Assign_expression,
      Call_expression,
      Block_expression,
      If_expression
    >
      value;
  };

  // statements

  struct Let_statement
  {
    std::size_t index;
    Expression initializer;
  };

  struct While_statement
  {
    ~While_statement();

    While_statement() = default;

    While_statement(While_statement &&) noexcept = default;

    While_statement &operator=(While_statement &&) noexcept = default;

    std::unique_ptr<Expression> condition;
    Block_expression body;
  };

  struct Return_statement
  {
    Expression value;
  };

  struct Expression_statement
  {
    Expression expression;
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

  // program

  struct Function
  {
    std::optional<std::string> name;
    std::size_t parameter_count;
    std::vector<std::string> local_names;
    Block_expression body;
  };

  struct Program
  {
    std::vector<Function> functions;
  };

} // namespace basedir

#endif // BASEDIR_IR_H
