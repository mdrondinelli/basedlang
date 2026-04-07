#include <cassert>
#include <cctype>
#include <string>

#include "basedlex/lexeme_stream.h"

namespace basedlex
{

  Lexeme_stream::Lexeme_stream(Char_stream *stream) noexcept
      : _reader{stream}, _location{.line = 1, .column = 1}
  {
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
    auto text = std::string{};
    auto token_location = Source_location{};
    auto state = 0;
    for (;;)
    {
      switch (state)
      {
      case 0:
        {
          if (!_reader.peek())
          {
            return Lexeme{
              .text = "",
              .token = Token::eof,
              .location = _location,
            };
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
            text += (char) c;
            state = 1;
            break;
          }
          if (c <= 0x7F && std::isdigit((int) c))
          {
            text += (char) c;
            state = 2;
            break;
          }
          if (c == '-')
          {
            if (_reader.peek() && *_reader.peek() == '>')
            {
              consume_non_newline();
              return Lexeme{
                .text = "->",
                .token = Token::arrow,
                .location = token_location,
              };
            }
            return Lexeme{
              .text = "-",
              .token = Token::minus,
              .location = token_location,
            };
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
                consume_non_newline();
                consume_non_newline();
                consume_non_newline();
                return Lexeme{
                  .text = "&mut",
                  .token = Token::ampersand_mut,
                  .location = token_location,
                };
              }
              return Lexeme{
                .text = "&",
                .token = Token::ampersand,
                .location = token_location
              };
            }
          case '+':
            return Lexeme{
              .text = "+",
              .token = Token::plus,
              .location = token_location
            };
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
                consume_non_newline();
                consume_non_newline();
                consume_non_newline();
                return Lexeme{
                  .text = "^mut",
                  .token = Token::caret_mut,
                  .location = token_location,
                };
              }
              return Lexeme{
                .text = "^",
                .token = Token::caret,
                .location = token_location
              };
            }
          case '*':
            return Lexeme{
              .text = "*",
              .token = Token::star,
              .location = token_location
            };
          case '/':
            return Lexeme{
              .text = "/",
              .token = Token::slash,
              .location = token_location
            };
          case '%':
            return Lexeme{
              .text = "%",
              .token = Token::percent,
              .location = token_location
            };
          case '=':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              consume_non_newline();
              return Lexeme{
                .text = "==",
                .token = Token::eq_eq,
                .location = token_location,
              };
            }
            if (_reader.peek() && *_reader.peek() == '>')
            {
              consume_non_newline();
              return Lexeme{
                .text = "=>",
                .token = Token::fat_arrow,
                .location = token_location,
              };
            }
            return Lexeme{
              .text = "=",
              .token = Token::eq,
              .location = token_location,
            };
          case '!':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              consume_non_newline();
              return Lexeme{
                .text = "!=",
                .token = Token::bang_eq,
                .location = token_location
              };
            }
            throw Lex_error{token_location};
          case '<':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              consume_non_newline();
              return Lexeme{
                .text = "<=",
                .token = Token::le,
                .location = token_location
              };
            }
            return Lexeme{
              .text = "<",
              .token = Token::lt,
              .location = token_location
            };
          case '>':
            if (_reader.peek() && *_reader.peek() == '=')
            {
              consume_non_newline();
              return Lexeme{
                .text = ">=",
                .token = Token::ge,
                .location = token_location
              };
            }
            return Lexeme{
              .text = ">",
              .token = Token::gt,
              .location = token_location
            };
          case '{':
            return Lexeme{
              .text = "{",
              .token = Token::lbrace,
              .location = token_location
            };
          case '[':
            return Lexeme{
              .text = "[",
              .token = Token::lbracket,
              .location = token_location
            };
          case '}':
            return Lexeme{
              .text = "}",
              .token = Token::rbrace,
              .location = token_location
            };
          case ']':
            return Lexeme{
              .text = "]",
              .token = Token::rbracket,
              .location = token_location
            };
          case '(':
            return Lexeme{
              .text = "(",
              .token = Token::lparen,
              .location = token_location
            };
          case ')':
            return Lexeme{
              .text = ")",
              .token = Token::rparen,
              .location = token_location
            };
          case ':':
            return Lexeme{
              .text = ":",
              .token = Token::colon,
              .location = token_location
            };
          case ',':
            return Lexeme{
              .text = ",",
              .token = Token::comma,
              .location = token_location
            };
          case ';':
            return Lexeme{
              .text = ";",
              .token = Token::semicolon,
              .location = token_location
            };
          default:
            throw Lex_error{token_location};
          }
        }
      case 1:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && (std::isalnum((int) *p) || *p == '_'))
          {
            text += (char) consume_non_newline();
            break;
          }
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
          return Lexeme{
            .text = text,
            .token = token,
            .location = token_location,
          };
        }
      case 2:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && std::isdigit((int) *p))
          {
            text += (char) consume_non_newline();
            break;
          }
          if (p && *p == '.')
          {
            auto const p1 = _reader.peek(1);
            if (p1 && *p1 <= 0x7F && std::isdigit((int) *p1))
            {
              text += (char) consume_non_newline();
              state = 3;
              break;
            }
          }
          if (p && *p == 'i')
          {
            text += (char) consume_non_newline();
            auto suffix_digit_count = 0;
            for (;;)
            {
              auto const suffix_peek = _reader.peek();
              if (!suffix_peek || *suffix_peek > 0x7F ||
                  !std::isdigit((int) *suffix_peek))
              {
                break;
              }
              text += (char) consume_non_newline();
              ++suffix_digit_count;
            }
            if (suffix_digit_count == 0)
            {
              throw Lex_error{token_location};
            }
          }
          return Lexeme{
            .text = text,
            .token = Token::int_literal,
            .location = token_location,
          };
        }
      case 3:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && std::isdigit((int) *p))
          {
            text += (char) consume_non_newline();
            break;
          }
          if (p && *p == 'f')
          {
            text += (char) consume_non_newline();
            auto suffix_digit_count = 0;
            for (;;)
            {
              auto const suffix_peek = _reader.peek();
              if (!suffix_peek || *suffix_peek > 0x7F ||
                  !std::isdigit((int) *suffix_peek))
              {
                break;
              }
              text += (char) consume_non_newline();
              ++suffix_digit_count;
            }
            if (suffix_digit_count == 0)
            {
              throw Lex_error{token_location};
            }
          }
          return Lexeme{
            .text = text,
            .token = Token::float_literal,
            .location = token_location,
          };
        }
      }
    }
  }

} // namespace basedlex
