# Architecture and pipeline overview

bensonlang currently has five main modules:

1. `lexing`
2. `ast`
3. `parsing`
4. `ir`
5. `benson`

The pipeline is simple:

1. bytes are decoded into Unicode characters
2. characters are lexed into `Lexeme` values
3. lexemes are parsed into an AST (defined by `ast`, produced by `parsing`)
4. the AST is compiled into HLIR with name resolution, type evaluation, diagnostics, and constant evaluation
5. the `benson` executable can currently interpret a chosen HLIR function

## Module boundaries

### `lexing`

Owns tokenization, source locations, and character/token lookahead.

### `ast`

Owns the AST data model: expression and statement node types, operator identity, and source spans over AST nodes.

### `parsing`

Owns syntax: consuming lexemes and constructing the `ast` data model. Owns precedence rules.

### `ir`

Owns semantics:

- symbol resolution
- type identity
- compile-time values
- diagnostics
- lowering to HLIR
- HLIR interpretation

### `benson`

Owns command-line wiring. It is basically a throwaway tool at this point.

## The main type-system fact

Types are ordinary compile-time values in the language. There is no separate implementation world where type syntax is fundamentally different from other expressions.

That is why semantic code has to support:

- builtin names like `Int32`, `Bool`, and `Void`
- type-valued expressions
- canonicalized `Type *` identity

## The main execution fact

HLIR is the executable representation produced by compilation. It should remain
self-contained. Today it is directly interpreted for development convenience,
but that should not be treated as the long-term product direction.

## How to use this wiki

- Start here for the overall shape.
- Then read the module page for the code you are changing or reviewing.
- Use [Tutorial: first change](./tutorial-first-change.md) if you are starting from zero.
