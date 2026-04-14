# Project structure

This page describes the repository in stable conceptual terms. It intentionally avoids depending on individual source filenames.

## Main code areas

### `code/lexing`

The lexer library. Owns character input, buffering/lookahead, tokenization, and source-location tracking at the token level.

See [`lexing`](./lexing.md).

### `code/spelling`

Shared preserved-spelling storage for the front end. Owns `Spelling`,
`Spelling_table`, and the builder/finalize API used to construct and deduplicate
variable token spellings without per-lexeme owned strings.

See [`spelling`](./spelling.md).

### `code/ast`

The AST library. Owns the data model for all syntax forms: expression and statement node types, operator identity, and source-span utilities.

See [`ast`](./ast.md).

### `code/parsing`

The parser library. Owns expression precedence, statement parsing, and AST construction via the types defined in `ast`.

See [`parsing`](./parsing.md).

### `code/ir`

The semantic and execution core. Owns:

- types
- symbol resolution
- diagnostics
- compile-time values
- lowering to HLIR
- interpretation of HLIR

See [`ir`](./ir.md).

### `code/benson`

The CLI entrypoint.

See [`benson`](./benson.md).

## Supporting areas

### `examples`

Small source programs used as runnable examples and as useful behavioral anchors when developing features.

### Tests inside each library

Tests live next to the libraries they validate. This is important because most changes should have a clear ownership boundary:

- lexical behavior belongs with lexer tests
- syntax behavior belongs with parser tests
- semantic and lowering behavior belongs with HLIR tests

For the local build and test workflow, see [Tutorial: first change](./tutorial-first-change.md).

## Mental map for contributors

When deciding where a change belongs, ask this in order:

1. Is this about raw characters or token boundaries?
2. Is this about AST node shapes, operator identity, or source spans?
3. Is this about syntax only — consuming tokens to produce AST nodes?
4. Is this about meaning, type checking, lowering, or diagnostics?
5. Is this only about wiring the executable?

Those map directly to the main modules, with shared preserved-spelling storage
living between lexing and later front-end consumers.

## Stability guidelines

The repo may be reorganized internally over time. The stable structure to preserve in docs and reviews is:

- lexer
- spelling storage
- AST data model
- parser
- semantic compiler / HLIR
- executable wrapper

If that structure changes, this page and the overview/module pages should change in the same PR.
