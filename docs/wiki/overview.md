# Architecture and pipeline overview

basedlang currently has five main modules:

1. `basedlex`
2. `basedast`
3. `basedparse`
4. `basedhlir`
5. `based`

The pipeline is simple:

1. bytes are decoded into Unicode characters
2. characters are lexed into `Lexeme` values
3. lexemes are parsed into an AST (defined by `basedast`, produced by `basedparse`)
4. the AST is compiled into HLIR with name resolution, type evaluation, diagnostics, and constant evaluation
5. the `based` executable can currently interpret a chosen HLIR function

## Module boundaries

### `basedlex`

Owns tokenization, source locations, and character/token lookahead.

### `basedast`

Owns the AST data model: expression and statement node types, operator identity, and source spans over AST nodes.

### `basedparse`

Owns syntax: consuming lexemes and constructing the `basedast` data model. Owns precedence rules.

### `basedhlir`

Owns semantics:

- symbol resolution
- type identity
- compile-time values
- diagnostics
- lowering to HLIR
- HLIR interpretation

### `based`

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
