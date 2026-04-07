# `basedparse`

`basedparse` is the parser module.

## Interface

The public surface is centered on:

- `Parser`

This module consumes lexemes and produces the AST defined in [`basedast`](./basedast.md).

## Core abstractions

### `Parser`

Hand-written parser over `Lexeme_stream_reader`. Produces `Translation_unit`, `Expression`, and `Statement` values from `basedast`.

## Algorithm

The parser is hand-written recursive descent.

Most syntax forms are parsed by:

1. inspecting the next lexeme
2. choosing the parse function for the construct
3. consuming required lexemes with `expect(...)`
4. recursively parsing child nodes
5. building an explicit AST node

Expression parsing uses precedence climbing. The parser reads a primary expression and then keeps extending it while the next postfix or binary operator has enough precedence to continue.

## What to keep stable

- parsing stays syntax-only
- AST nodes must preserve all lexemes

## See also

- [`basedast`](./basedast.md) — data model produced by this module
