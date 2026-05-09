# Architecture and pipeline overview

bensonlang currently has twelve main modules:

1. `source`
2. `streams`
3. `lexing`
4. `spelling`
5. `ast`
6. `parsing`
7. `hlir`
8. `frontend`
9. `llir`
10. `bytecode`
11. `vm`
12. `benson`

The pipeline is simple:

1. bytes are decoded into Unicode characters
2. characters are lexed into `Lexeme` values
3. preserved variable spellings can be interned into `Spelling_table` storage
4. lexemes are parsed into an AST (defined by `ast`, produced by `parsing`)
5. the AST is compiled by `frontend` into HLIR (the `hlir` data model) with name resolution, type evaluation, diagnostics, and constant evaluation
6. the `benson` executable can currently interpret a chosen HLIR function using the `hlir` interpreter
7. LLIR, bytecode, and VM libraries are under development as a lower-level execution path

## Module boundaries

### `source`

Owns source coordinate data: `Source_location`, `Source_span`, and span hull
helpers.

### `lexing`

Owns tokenization, source-location tracking at lex time, and token lookahead.

### `streams`

Owns raw byte/character input, UTF-8 decoding, and character lookahead.

### `spelling`

Owns shared preserved-spelling storage used by later front-end phases. It
provides compact `Spelling` handles and a `Spelling_table` builder/finalize
API for incremental token-text construction.

### `ast`

Owns the AST data model: expression and statement node types, operator identity, and `span_of` accessors over AST nodes.

### `parsing`

Owns syntax: consuming lexemes and constructing the `ast` data model. Owns precedence rules.

### `hlir`

Owns the HLIR data model and its interpreter:

- type identity (`Type`, `Type_pool`)
- compile-time values (`Constant_value`)
- executable HLIR (`Register`, `Operand`, `Instruction`, `Terminator`, `Basic_block`, `Function`, `Translation_unit`)
- HLIR interpretation

The `hlir` library does not depend on `ast`. Any module that only needs to hold or execute HLIR can link it in isolation.

### `frontend`

Owns AST→HLIR lowering:

- name resolution (`Symbol_table`)
- type evaluation
- operator overload resolution
- diagnostics
- emission of HLIR (via `Frontend`, `compile`)

Depends on both `ast` and `hlir`.

### `llir`

Owns the low-level IR data model that will sit between HLIR and bytecode:

- virtual registers with explicit register types
- arbitrary-width bit registers
- block parameters and structured control flow

It depends on `spelling`, but not on frontend, HLIR, bytecode, or VM details.

### `bytecode`

Owns the low-level bytecode representation:

- opcodes and registers
- narrow and wide operands
- constants
- module construction and byte emission

This is not currently documented as a stable binary format.

### `vm`

Owns execution of bytecode modules:

- VM register storage
- stack and constant address spaces
- bytecode dispatch
- instruction behavior

It depends on `bytecode`, but should stay independent from front-end and HLIR
details.

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

LLIR, bytecode, and the VM are a separate lower-level execution path. They are
still subject to change and should be reviewed as implementation
infrastructure.

## How to use this wiki

- Start here for the overall shape.
- Then read the module page for the code you are changing or reviewing.
- Use [Tutorial: first change](./tutorial-first-change.md) if you are starting from zero.
