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

  std::optional<Operator> get_prefix_operator(bensonlex::Token token)
  {
    switch (token)
    {
    case bensonlex::Token::ampersand:
      return Operator::address_of;
    case bensonlex::Token::ampersand_mut:
      return Operator::address_of_mut;
    case bensonlex::Token::caret:
      return Operator::pointer_to;
    case bensonlex::Token::caret_mut:
      return Operator::pointer_to_mut;
    case bensonlex::Token::plus:
      return Operator::unary_plus;
    case bensonlex::Token::minus:
      return Operator::unary_minus;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_postfix_operator(bensonlex::Token token)
  {
    switch (token)
    {
    case bensonlex::Token::lparen:
      return Operator::call;
    case bensonlex::Token::lbracket:
      return Operator::index;
    case bensonlex::Token::caret:
      return Operator::dereference;
    default:
      return std::nullopt;
    }
  }

  std::optional<Operator> get_binary_operator(bensonlex::Token token)
  {
    switch (token)
    {
    case bensonlex::Token::star:
      return Operator::multiply;
    case bensonlex::Token::slash:
      return Operator::divide;
    case bensonlex::Token::percent:
      return Operator::modulo;
    case bensonlex::Token::plus:
      return Operator::add;
    case bensonlex::Token::minus:
      return Operator::subtract;
    case bensonlex::Token::lt:
      return Operator::less;
    case bensonlex::Token::le:
      return Operator::less_eq;
    case bensonlex::Token::gt:
      return Operator::greater;
    case bensonlex::Token::ge:
      return Operator::greater_eq;
    case bensonlex::Token::eq_eq:
      return Operator::equal;
    case bensonlex::Token::bang_eq:
      return Operator::not_equal;
    case bensonlex::Token::eq:
      return Operator::assign;
    default:
      return std::nullopt;
    }
  }

} // namespace benson::ast
