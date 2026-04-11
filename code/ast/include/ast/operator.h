#ifndef BASEDAST_OPERATOR_H
#define BASEDAST_OPERATOR_H

#include <optional>

#include "lexing/token.h"

namespace benson::ast
{

  enum class Operator
  {
    call,
    index,
    dereference,
    address_of,
    address_of_mut,
    pointer_to,
    pointer_to_mut,
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

  std::optional<Operator> get_prefix_operator(Token token);

  std::optional<Operator> get_postfix_operator(Token token);

  std::optional<Operator> get_binary_operator(Token token);

} // namespace benson::ast

#endif // BASEDAST_OPERATOR_H
