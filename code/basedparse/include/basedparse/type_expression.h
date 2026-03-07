#ifndef BASEDPARSE_TYPE_EXPRESSION_H
#define BASEDPARSE_TYPE_EXPRESSION_H

#include "basedlex/lexeme.h"

namespace basedparse
{

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

} // namespace basedparse

#endif // BASEDPARSE_TYPE_EXPRESSION_H
