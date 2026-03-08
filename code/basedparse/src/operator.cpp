#include "basedparse/operator.h"

namespace basedparse
{

  int get_operator_precedence(Operator op)
  {
    switch (op)
    {
    case Operator::call:
      return 0;
    case Operator::multiply:
    case Operator::divide:
    case Operator::modulo:
      return 1;
    case Operator::add:
    case Operator::subtract:
      return 2;
    }
  }

  std::optional<Operator> get_binary_operator(basedlex::Token token)
  {
    switch (token)
    {
    case basedlex::Token::star:
      return Operator::multiply;
    case basedlex::Token::slash:
      return Operator::divide;
    case basedlex::Token::percent:
      return Operator::modulo;
    case basedlex::Token::plus:
      return Operator::add;
    case basedlex::Token::minus:
      return Operator::subtract;
    default:
      return std::nullopt;
    }
  }

} // namespace basedparse
