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
    eq, // =
    lbrace, // {
    lparen, // (
    rbrace, // }
    rparen, // )
    comma, // ,
    semicolon, // ;
  };

} // namespace basedlex

#endif // BASEDLEX_TOKEN_H
