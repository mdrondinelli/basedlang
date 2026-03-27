#ifndef BASEDLEX_TOKEN_H
#define BASEDLEX_TOKEN_H

namespace basedlex
{

  enum class Token
  {
    eof,
    identifier,
    int_literal,
    kw_else,
    kw_fn,
    kw_if,
    kw_let,
    kw_mut,
    kw_return,
    kw_while,
    arrow, // ->
    fat_arrow, // =>
    colon, // :
    plus, // +
    minus, // -
    star, // *
    star_mut, // *mut
    ampersand, // &
    ampersand_mut, // &mut
    slash, // /
    percent, // %
    eq, // =
    eq_eq, // ==
    bang_eq, // !=
    lt, // <
    le, // <=
    gt, // >
    ge, // >=
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
