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
    address_of,
    address_of_mut,
    dereference,
    unary_plus,
    unary_minus,
    multiply,
    divide,
    modulo,
    add,
    subtract,
    less,
    less_eq,
    greater,
    greater_eq,
    equal,
    not_equal,
    assign,
  };

  enum class Operator_associativity
  {
    left,
    right,
  };

  int get_operator_precedence(Operator op);

  Operator_associativity get_precedence_associativity(int precedence);

  std::optional<Operator> get_unary_operator(basedlex::Token token);

  std::optional<Operator> get_postfix_operator(basedlex::Token token);

  std::optional<Operator> get_binary_operator(basedlex::Token token);

} // namespace basedparse

#endif // BASEDPARSE_OPERATOR_H
