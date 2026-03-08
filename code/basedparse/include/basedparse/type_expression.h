#ifndef BASEDPARSE_TYPE_EXPRESSION_H
#define BASEDPARSE_TYPE_EXPRESSION_H

#include <memory>

#include "basedlex/lexeme.h"

namespace basedparse
{

  class Expression;

  class Type_expression
  {
  public:
    virtual ~Type_expression() = default;
  };

  class Identifier_type_expression: public Type_expression
  {
  public:
    basedlex::Lexeme identifier;
  };

  class Array_type_expression: public Type_expression
  {
  public:
    ~Array_type_expression() override;

    std::unique_ptr<Type_expression> element_type;
    basedlex::Lexeme lbracket;
    std::unique_ptr<Expression> size;
    basedlex::Lexeme rbracket;
  };

} // namespace basedparse

#endif // BASEDPARSE_TYPE_EXPRESSION_H
