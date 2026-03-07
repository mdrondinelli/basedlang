#ifndef BASEDPARSE_EXPRESSION_H
#define BASEDPARSE_EXPRESSION_H

#include <memory>

#include "basedlex/lexeme.h"

#include "type_expression.h"

namespace basedparse
{

  class Expression
  {
  public:
    virtual ~Expression() = default;
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
    Fn_expression() = default;
    ~Fn_expression() override;
    Fn_expression(Fn_expression &&) noexcept = default;
    Fn_expression &operator=(Fn_expression &&) noexcept = default;
    basedlex::Lexeme kw_fn;
    basedlex::Lexeme lparen;
    // TODO: parameters
    basedlex::Lexeme rparen;
    basedlex::Lexeme arrow;
    std::unique_ptr<Type_expression> return_type;
    std::unique_ptr<class Block_statement> body;
  };

} // namespace basedparse

#endif // BASEDPARSE_EXPRESSION_H
