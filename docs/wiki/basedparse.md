# `basedparse`

`basedparse` is the parser module.

## Interface

The public surface is centered on:

- `Parser`
- `Expression`
- `Statement`
- `Translation_unit`
- `Operator`
- `Source_span`

This module consumes lexemes and produces source-preserving AST nodes.

## Core abstractions

### `Parser`

Hand-written parser over `Lexeme_stream_reader`.

### `Expression`

The main AST sum type for expression forms.

### `Statement`

The AST sum type for statements inside blocks.

### `Translation_unit`

The AST for the whole program.

### `Operator`

Semantic operator identity used for precedence and later compilation logic.

### `Source_span`

Source range over lexemes or AST nodes.

## Data model

The AST is explicit:

- each syntax form has its own struct
- lexemes are preserved throughout the tree
- recursive links use `std::unique_ptr`
- sum types use `std::variant`

That keeps diagnostics source-facing and makes later compilation code easy to line up with syntax.

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
- precedence and associativity changes must stay coherent
- AST nodes must preserve the lexemes needed for diagnostics

