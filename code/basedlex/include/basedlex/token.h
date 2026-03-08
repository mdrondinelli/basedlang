#ifndef BASEDLEX_TOKEN_H
#define BASEDLEX_TOKEN_H

namespace basedlex
{

  enum class Token
  {
    eof,
    identifier,
    int_literal,
    kw_fn,
    kw_let,
    kw_return,
    arrow, // ->
    colon, // :
    plus, // +
    minus, // -
    star, // *
    slash, // /
    percent, // %
    eq, // =
    lbrace, // {
    lbracket, // [
    lparen, // (
    rbrace, // }
    rbracket, // ]
    rparen, // )
    comma, // ,
    semicolon, // ;
  };

} // namespace basedlex

#endif // BASEDLEX_TOKEN_H
