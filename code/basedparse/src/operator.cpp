#include "basedparse/operator.h"

namespace basedparse
{

  int get_operator_precedence(Operator op)
  {
    switch (op)
    {
    case Operator::call:
    case Operator::index:
      return 0;
    case Operator::unary_plus:
    case Operator::unary_minus:
      return 1;
    case Operator::multiply:
    case Operator::divide:
    case Operator::modulo:
      return 2;
    case Operator::add:
    case Operator::subtract:
      return 3;
    }
  }

  std::optional<Operator> get_unary_operator(basedlex::Token token)
  {
    switch (token)
    {
    case basedlex::Token::plus:
      return Operator::unary_plus;
    case basedlex::Token::minus:
      return Operator::unary_minus;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_postfix_operator(basedlex::Token token)
  {
    switch (token)
    {
    case basedlex::Token::lparen:
      return Operator::call;
    case basedlex::Token::lbracket:
      return Operator::index;
    default:
      return std::nullopt;
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
