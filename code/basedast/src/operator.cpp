#include "basedast/operator.h"

namespace basedparse
{

  int get_operator_precedence(Operator op)
  {
    switch (op)
    {
    case Operator::call:
    case Operator::index:
    case Operator::dereference:
      return 0;
    case Operator::address_of:
    case Operator::address_of_mut:
    case Operator::pointer_to:
    case Operator::pointer_to_mut:
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
    case Operator::less:
    case Operator::less_eq:
    case Operator::greater:
    case Operator::greater_eq:
      return 4;
    case Operator::equal:
    case Operator::not_equal:
      return 5;
    case Operator::assign:
      return 6;
    }
  }

  Operator_associativity get_precedence_associativity(int precedence)
  {
    switch (precedence)
    {
    case 6: // assign
      return Operator_associativity::right;
    default:
      return Operator_associativity::left;
    }
  }

  std::optional<Operator> get_prefix_operator(basedlex::Token token)
  {
    switch (token)
    {
    case basedlex::Token::ampersand:
      return Operator::address_of;
    case basedlex::Token::ampersand_mut:
      return Operator::address_of_mut;
    case basedlex::Token::caret:
      return Operator::pointer_to;
    case basedlex::Token::caret_mut:
      return Operator::pointer_to_mut;
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
    case basedlex::Token::caret:
      return Operator::dereference;
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
    case basedlex::Token::lt:
      return Operator::less;
    case basedlex::Token::le:
      return Operator::less_eq;
    case basedlex::Token::gt:
      return Operator::greater;
    case basedlex::Token::ge:
      return Operator::greater_eq;
    case basedlex::Token::eq_eq:
      return Operator::equal;
    case basedlex::Token::bang_eq:
      return Operator::not_equal;
    case basedlex::Token::eq:
      return Operator::assign;
    default:
      return std::nullopt;
    }
  }

} // namespace basedparse
