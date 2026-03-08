#ifndef BASEDPARSE_EXPRESSION_H
#define BASEDPARSE_EXPRESSION_H

#include <memory>
#include <optional>
#include <vector>

#include "basedlex/lexeme.h"

#include "type_expression.h"

namespace basedparse
{

  class Expression
  {
  public:
    virtual ~Expression() = default;
  };

  struct Parameter
  {
    basedlex::Lexeme name;
    basedlex::Lexeme colon;
    std::unique_ptr<Type_expression> type_expression;
  };

  class Int_literal_expression: public Expression
  {
  public:
    basedlex::Lexeme literal;
  };

  class Identifier_expression: public Expression
  {
  public:
    basedlex::Lexeme identifier;
  };

  class Fn_expression: public Expression
  {
  public:
    struct Return_type_specifier
    {
      basedlex::Lexeme arrow;
      std::unique_ptr<Type_expression> type_expression;
    };

    Fn_expression() = default;

    ~Fn_expression() override;

    Fn_expression(Fn_expression &&) noexcept = default;

    Fn_expression &operator=(Fn_expression &&) noexcept = default;

    basedlex::Lexeme kw_fn;
    basedlex::Lexeme lparen;
    std::vector<Parameter> parameters;
    std::vector<basedlex::Lexeme> parameter_commas;
    basedlex::Lexeme rparen;
    std::optional<Return_type_specifier> return_type_specifier;
    std::unique_ptr<class Block_statement> body;
  };

  class Paren_expression: public Expression
  {
  public:
    basedlex::Lexeme lparen;
    std::unique_ptr<Expression> inner;
    basedlex::Lexeme rparen;
  };

  class Call_expression: public Expression
  {
  public:
    std::unique_ptr<Expression> callee;
    basedlex::Lexeme lparen;
    std::vector<std::unique_ptr<Expression>> arguments;
    std::vector<basedlex::Lexeme> argument_commas;
    basedlex::Lexeme rparen;
  };

} // namespace basedparse

#endif // BASEDPARSE_EXPRESSION_H
