#include "lexing/lexeme_stream.h"
#include <cassert>
#include <cctype>

namespace benson
{

  Lexeme_stream::Lexeme_stream(
    Char_stream *stream,
    Spelling_table *spellings
  ) noexcept
      : _reader{stream},
        _spellings{spellings},
        _location{.line = 1, .column = 1}
  {
    assert(_spellings != nullptr);
  }

  void Lexeme_stream::consume_newline()
  {
    assert(_reader.peek() && *_reader.peek() == '\n');
    _reader.read();
    ++_location.line;
    _location.column = 1;
  }

  char32_t Lexeme_stream::consume_non_newline()
  {
    assert(_reader.peek() && *_reader.peek() != '\n');
    auto const c = *_reader.read();
    ++_location.column;
    return c;
  }

  Lexeme Lexeme_stream::lex()
  {
    auto token_location = Source_location{};
    auto build = _spellings->builder();
    auto const append = [&](char32_t c)
    {
      build.append(static_cast<char>(c));
    };
    auto const make_lexeme =
      [&](Token token, Source_location start, Spelling spelling = {}) -> Lexeme
    {
      auto end = start;
      if (token != Token::eof)
      {
        end = Source_location{
          .line = _location.line,
          .column = _location.column - 1,
        };
        if (!spelling.has_value())
        {
          spelling = std::move(build).finalize();
        }
      }
      return Lexeme{
        .token = token,
        .spelling = spelling,
        .span = Source_span{.start = start, .end = end},
      };
    };
    auto state = 0;
    for (;;)
    {
      switch (state)
      {
      case 0:
        {
          if (!_reader.peek())
          {
            return make_lexeme(Token::eof, _location);
          }
          if (*_reader.peek() == '\n')
          {
            consume_newline();
            break;
          }
          auto const c_location = _location;
          auto const c = consume_non_newline();
          if (c == ' ' || c == '\t' || c == '\r')
          {
            break;
          }
          token_location = c_location;
          if (c <= 0x7F && (std::isalpha((int) c) || c == '_'))
          {
            append(c);
            state = 1;
            break;
          }
          if (c <= 0x7F && std::isdigit((int) c))
          {
            append(c);
            state = 2;
            break;
          }
          if (c == '-')
          {
            if (_reader.peek() && *_reader.peek() == '>')
            {
              append(c);
              append(consume_non_newline());
              return make_lexeme(Token::arrow, token_location);
            }
            append(c);
            return make_lexeme(Token::minus, token_location);
          }
          switch (c)
          {
          case '&':
            {
              auto const p0 = _reader.peek(0);
              auto const p1 = _reader.peek(1);
              auto const p2 = _reader.peek(2);
              auto const p3 = _reader.peek(3);
              if (p0 && *p0 == 'm' && p1 && *p1 == 'u' && p2 && *p2 == 't' &&
                  (!p3 ||
                   !(*p3 <= 0x7F && (std::isalnum((int) *p3) || *p3 == '_'))))
              {
                append(c);
                append(consume_non_newline());
                append(consume_non_newline());
                append(consume_non_newline());
                return make_lexeme(Token::ampersand_mut, token_location);
              }
              append(c);
              return make_lexeme(Token::ampersand, token_location);
            }
          case '+':
            append(c);
            return make_lexeme(Token::plus, token_location);
          case '^':
            {
              auto const p0 = _reader.peek(0);
              auto const p1 = _reader.peek(1);
              auto const p2 = _reader.peek(2);
              auto const p3 = _reader.peek(3);
              if (p0 && *p0 == 'm' && p1 && *p1 == 'u' && p2 && *p2 == 't' &&
                  (!p3 ||
                   !(*p3 <= 0x7F && (std::isalnum((int) *p3) || *p3 == '_'))))
              {
                append(c);
                append(consume_non_newline());
                append(consume_non_newline());
                append(consume_non_newline());
                return make_lexeme(Token::caret_mut, token_location);
              }
              append(c);
              return make_lexeme(Token::caret, token_location);
            }
          case '*':
            append(c);
            return make_lexeme(Token::star, token_location);
          case '/':
            append(c);
            return make_lexeme(Token::slash, token_location);
          case '%':
            append(c);
            return make_lexeme(Token::percent, token_location);
          case '=':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              append(c);
              append(consume_non_newline());
              return make_lexeme(Token::eq_eq, token_location);
            }
            if (_reader.peek() && *_reader.peek() == '>')
            {
              append(c);
              append(consume_non_newline());
              return make_lexeme(Token::fat_arrow, token_location);
            }
            append(c);
            return make_lexeme(Token::eq, token_location);
          case '!':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              append(c);
              append(consume_non_newline());
              return make_lexeme(Token::bang_eq, token_location);
            }
            throw Lex_error{token_location};
          case '<':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              append(c);
              append(consume_non_newline());
              return make_lexeme(Token::le, token_location);
            }
            append(c);
            return make_lexeme(Token::lt, token_location);
          case '>':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              append(c);
              append(consume_non_newline());
              return make_lexeme(Token::ge, token_location);
            }
            append(c);
            return make_lexeme(Token::gt, token_location);
          case '{':
            append(c);
            return make_lexeme(Token::lbrace, token_location);
          case '[':
            append(c);
            return make_lexeme(Token::lbracket, token_location);
          case '}':
            append(c);
            return make_lexeme(Token::rbrace, token_location);
          case ']':
            append(c);
            return make_lexeme(Token::rbracket, token_location);
          case '(':
            append(c);
            return make_lexeme(Token::lparen, token_location);
          case ')':
            append(c);
            return make_lexeme(Token::rparen, token_location);
          case ':':
            append(c);
            return make_lexeme(Token::colon, token_location);
          case ',':
            append(c);
            return make_lexeme(Token::comma, token_location);
          case ';':
            append(c);
            return make_lexeme(Token::semicolon, token_location);
          default:
            throw Lex_error{token_location};
          }
        }
      case 1:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && (std::isalnum((int) *p) || *p == '_'))
          {
            append(consume_non_newline());
            break;
          }
          auto const spelling = std::move(build).finalize();
          auto const text = _spellings->lookup(spelling);
          auto const token = [&]() -> Token
          {
            if (text == "else")
            {
              return Token::kw_else;
            }
            if (text == "fn")
            {
              return Token::kw_fn;
            }
            if (text == "if")
            {
              return Token::kw_if;
            }
            if (text == "let")
            {
              return Token::kw_let;
            }
            if (text == "mut")
            {
              return Token::kw_mut;
            }
            if (text == "recurse")
            {
              return Token::kw_recurse;
            }
            if (text == "return")
            {
              return Token::kw_return;
            }
            if (text == "while")
            {
              return Token::kw_while;
            }
            return Token::identifier;
          }();
          return make_lexeme(token, token_location, spelling);
        }
      case 2:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && std::isdigit((int) *p))
          {
            append(consume_non_newline());
            break;
          }
          if (p && *p == '.')
          {
            append(consume_non_newline());
            state = 3;
            break;
          }
          if (p && (*p == 'f' || *p == 'd'))
          {
            append(consume_non_newline());
            return make_lexeme(Token::float_literal, token_location);
          }
          if (p && *p == 'i')
          {
            append(consume_non_newline());
            auto suffix_digit_count = 0;
            for (;;)
            {
              auto const suffix_peek = _reader.peek();
              if (!suffix_peek || *suffix_peek > 0x7F ||
                  !std::isdigit((int) *suffix_peek))
              {
                break;
              }
              append(consume_non_newline());
              ++suffix_digit_count;
            }
            if (suffix_digit_count == 0)
            {
              throw Lex_error{token_location};
            }
          }
          return make_lexeme(Token::int_literal, token_location);
        }
      case 3:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && std::isdigit((int) *p))
          {
            append(consume_non_newline());
            break;
          }
          if (p && (*p == 'f' || *p == 'd'))
          {
            append(consume_non_newline());
          }
          return make_lexeme(Token::float_literal, token_location);
        }
      }
    }
  }

} // namespace benson
