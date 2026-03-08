#include <cctype>
#include <string>

#include "basedlex/lexeme_stream.h"

namespace basedlex
{

  Lexeme_stream::Lexeme_stream(Char_stream *stream) noexcept
      : _reader{stream}, _line{1}, _column{1}
  {
  }

  Lexeme Lexeme_stream::lex()
  {
    auto text = std::string{};
    auto token_line = int{};
    auto token_column = int{};
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
              .line = _line,
              .column = _column
            };
          }
          auto const c = *_reader.read();
          auto const c_line = _line;
          auto const c_col = _column;
          if (c == '\n')
          {
            ++_line;
            _column = 1;
            break;
          }
          ++_column;
          if (c == ' ' || c == '\t' || c == '\r')
          {
            break;
          }
          token_line = c_line;
          token_column = c_col;
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
              _reader.read();
              ++_column;
              return Lexeme{
                .text = "->",
                .token = Token::arrow,
                .line = token_line,
                .column = token_column
              };
            }
            throw Lex_error{token_line, token_column};
          }
          switch (c)
          {
          case '=':
            return Lexeme{
              .text = "=",
              .token = Token::eq,
              .line = token_line,
              .column = token_column
            };
          case '{':
            return Lexeme{
              .text = "{",
              .token = Token::lbrace,
              .line = token_line,
              .column = token_column
            };
          case '}':
            return Lexeme{
              .text = "}",
              .token = Token::rbrace,
              .line = token_line,
              .column = token_column
            };
          case '(':
            return Lexeme{
              .text = "(",
              .token = Token::lparen,
              .line = token_line,
              .column = token_column
            };
          case ')':
            return Lexeme{
              .text = ")",
              .token = Token::rparen,
              .line = token_line,
              .column = token_column
            };
          case ',':
            return Lexeme{
              .text = ",",
              .token = Token::comma,
              .line = token_line,
              .column = token_column
            };
          case ';':
            return Lexeme{
              .text = ";",
              .token = Token::semicolon,
              .line = token_line,
              .column = token_column
            };
          default:
            throw Lex_error{token_line, token_column};
          }
        }
      case 1:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && (std::isalnum((int) *p) || *p == '_'))
          {
            text += (char) *p;
            _reader.read();
            ++_column;
            break;
          }
          auto const token = [&]() -> Token
          {
            if (text == "fn")
            {
              return Token::kw_fn;
            }
            if (text == "let")
            {
              return Token::kw_let;
            }
            if (text == "return")
            {
              return Token::kw_return;
            }
            return Token::identifier;
          }();
          return Lexeme{
            .text = text,
            .token = token,
            .line = token_line,
            .column = token_column
          };
        }
      case 2:
        {
          auto const p = _reader.peek();
          if (p && *p <= 0x7F && std::isdigit((int) *p))
          {
            text += (char) *p;
            _reader.read();
            ++_column;
            break;
          }
          return Lexeme{
            .text = text,
            .token = Token::int_literal,
            .line = token_line,
            .column = token_column
          };
        }
      }
    }
  }

} // namespace basedlex
