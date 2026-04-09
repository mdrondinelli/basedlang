#include "bensonast/operator.h"

namespace benson::ast
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

  std::optional<Operator> get_prefix_operator(benson::Token token)
  {
    switch (token)
    {
    case benson::Token::ampersand:
      return Operator::address_of;
    case benson::Token::ampersand_mut:
      return Operator::address_of_mut;
    case benson::Token::caret:
      return Operator::pointer_to;
    case benson::Token::caret_mut:
      return Operator::pointer_to_mut;
    case benson::Token::plus:
      return Operator::unary_plus;
    case benson::Token::minus:
      return Operator::unary_minus;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_postfix_operator(benson::Token token)
  {
    switch (token)
    {
    case benson::Token::lparen:
      return Operator::call;
    case benson::Token::lbracket:
      return Operator::index;
    case benson::Token::caret:
      return Operator::dereference;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_binary_operator(benson::Token token)
  {
    switch (token)
    {
    case benson::Token::star:
      return Operator::multiply;
    case benson::Token::slash:
      return Operator::divide;
    case benson::Token::percent:
      return Operator::modulo;
    case benson::Token::plus:
      return Operator::add;
    case benson::Token::minus:
      return Operator::subtract;
    case benson::Token::lt:
      return Operator::less;
    case benson::Token::le:
      return Operator::less_eq;
    case benson::Token::gt:
      return Operator::greater;
    case benson::Token::ge:
      return Operator::greater_eq;
    case benson::Token::eq_eq:
      return Operator::equal;
    case benson::Token::bang_eq:
      return Operator::not_equal;
    case benson::Token::eq:
      return Operator::assign;
    default:
      return std::nullopt;
    }
  }

} // namespace benson::ast
