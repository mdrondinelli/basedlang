#ifndef BASEDPARSE_OPERATOR_H
#define BASEDPARSE_OPERATOR_H

#include <optional>

#include "basedlex/token.h"

namespace basedparse
{

  enum class Operator
  {
    call,
    index,
    dereference,
    unary_plus,
    unary_minus,
    multiply,
    divide,
    modulo,
    add,
    subtract,
  };

  int get_operator_precedence(Operator op);

  std::optional<Operator> get_unary_operator(basedlex::Token token);

  std::optional<Operator> get_postfix_operator(basedlex::Token token);

  std::optional<Operator> get_binary_operator(basedlex::Token token);

} // namespace basedparse

#endif // BASEDPARSE_OPERATOR_H
