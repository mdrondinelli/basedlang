# `lexing`

`lexing` is the lexer module.

## Interface

The public surface is centered on:

- `Char_stream`
- `Char_stream_reader`
- `Lexeme_stream`
- `Lexeme_stream_reader`
- `Token`
- `Lexeme`

The parser consumes this module through `Lexeme_stream_reader`.

## Core abstractions

### `Token`

The closed set of token kinds in the language.

### `Lexeme`

A token plus its preserved text payload, if any, and source span.

### `Char_stream`

Abstract source of Unicode codepoints.

### `Char_stream_reader`

Character-level buffered lookahead.

### `Lexeme_stream`

The actual lexer.

### `Lexeme_stream_reader`

Token-level buffered lookahead for the parser.

## Data model

The important lexer data is small:

- token kind
- preserved token text when the token kind requires it
- source span

The lexer does not build a larger intermediate structure.

## Algorithm

The lexer is hand-written and direct.

At a high level, `Lexeme_stream` does this each time a token is requested:

1. skip trivia like whitespace
2. inspect the next character
3. enter a large branch or switch-style dispatch on that character
4. consume the rest of the token
5. return a `Lexeme`

Integer literals may carry an optional size suffix — `i8`, `i16`, `i32`, or `i64` — that is lexed as part of the same token and determines the integer type.

Identifier-like sequences are consumed first and then classified as keywords or identifiers. Punctuation-heavy tokens use lookahead and longest-match behavior for things like `==`, `!=`, `<=`, `>=`, `=>`, `&mut`, and `^mut`.

The string-storage refactor is changing how token text is represented. Phase 2
introduced the shared `spelling` module, which provides `Spelling_table` and
`Spelling` for preserved variable-spelling tokens. Phase 3 will migrate
`Lexeme` away from owned text toward compact spelling handles for identifiers
and literals while keeping fixed-spelling tokens allocation-free.

## What to keep stable

- source locations must stay precise
- lexing stays incremental rather than tokenizing the whole file up front
