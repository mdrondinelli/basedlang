#include "kaleast/operator.h"

namespace kaleast
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

  std::optional<Operator> get_prefix_operator(kalelex::Token token)
  {
    switch (token)
    {
    case kalelex::Token::ampersand:
      return Operator::address_of;
    case kalelex::Token::ampersand_mut:
      return Operator::address_of_mut;
    case kalelex::Token::caret:
      return Operator::pointer_to;
    case kalelex::Token::caret_mut:
      return Operator::pointer_to_mut;
    case kalelex::Token::plus:
      return Operator::unary_plus;
    case kalelex::Token::minus:
      return Operator::unary_minus;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_postfix_operator(kalelex::Token token)
  {
    switch (token)
    {
    case kalelex::Token::lparen:
      return Operator::call;
    case kalelex::Token::lbracket:
      return Operator::index;
    case kalelex::Token::caret:
      return Operator::dereference;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_binary_operator(kalelex::Token token)
  {
    switch (token)
    {
    case kalelex::Token::star:
      return Operator::multiply;
    case kalelex::Token::slash:
      return Operator::divide;
    case kalelex::Token::percent:
      return Operator::modulo;
    case kalelex::Token::plus:
      return Operator::add;
    case kalelex::Token::minus:
      return Operator::subtract;
    case kalelex::Token::lt:
      return Operator::less;
    case kalelex::Token::le:
      return Operator::less_eq;
    case kalelex::Token::gt:
      return Operator::greater;
    case kalelex::Token::ge:
      return Operator::greater_eq;
    case kalelex::Token::eq_eq:
      return Operator::equal;
    case kalelex::Token::bang_eq:
      return Operator::not_equal;
    case kalelex::Token::eq:
      return Operator::assign;
    default:
      return std::nullopt;
    }
  }

} // namespace kaleast
