# Project structure

This page describes the repository in stable conceptual terms. It intentionally avoids depending on individual source filenames.

## Main code areas

### `code/source`

The header-only source-coordinate library. Owns `Source_location`,
`Source_span`, and helpers for composing source spans.

See [`source`](./source.md).

### `code/streams`

The raw input-stream library. Owns byte input, UTF-8 decoding, and
character-level buffering/lookahead.

See [`streams`](./streams.md).

### `code/lexing`

The lexer library. Owns tokenization and source-location tracking at the token
level.

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

### `code/hlir`

The HLIR model and interpreter. Owns:

- types
- compile-time values
- executable HLIR
- interpretation of HLIR

See [`hlir`](./hlir.md).

### `code/frontend`

The semantic compiler. Owns AST-to-HLIR lowering, name resolution, type
evaluation, operator resolution, and diagnostics.

See [`frontend`](./frontend.md).

### `code/llir`

The low-level IR data model. Owns bytecode-friendly virtual registers, register
types, block parameters, and structured control flow for the future
HLIR-to-bytecode path.

See [`llir`](./llir.md).

### `code/bytecode`

The bytecode library. Owns the low-level instruction representation, module
shape, bytecode emission, and module-building helpers.

See [`bytecode`](./bytecode.md).

### `code/vm`

The bytecode virtual machine. Owns bytecode module execution, VM register
storage, and runtime address-space handling.

See [`vm`](./vm.md).

### `code/benson`

The CLI entrypoint.

See [`benson`](./benson.md).

## Supporting areas

### `examples`

Small source programs used as runnable examples and as useful behavioral anchors when developing features.

### Tests inside each library

Tests live next to the libraries they validate. This is important because most changes should have a clear ownership boundary:

- raw stream and UTF-8 behavior belongs with `streams` tests
- lexical behavior belongs with lexer tests
- syntax behavior belongs with parser tests
- semantic and lowering behavior belongs with HLIR tests
- bytecode encoding belongs with bytecode tests
- bytecode execution belongs with VM tests

For the local build and test workflow, see [Tutorial: first change](./tutorial-first-change.md).

## Mental map for contributors

When deciding where a change belongs, ask this in order:

1. Is this about raw characters or token boundaries?
2. Is this about AST node shapes, operator identity, or source spans?
3. Is this about syntax only — consuming tokens to produce AST nodes?
4. Is this about meaning, type checking, lowering, or diagnostics?
5. Is this about HLIR data or HLIR interpretation?
6. Is this about low-level virtual registers or pre-bytecode control flow?
7. Is this about bytecode encoding or bytecode module construction?
8. Is this about running bytecode?
9. Is this only about wiring the executable?

Those map directly to the main modules, with shared preserved-spelling storage
living between lexing and later front-end consumers.

## Stability guidelines

The repo may be reorganized internally over time. The stable structure to preserve in docs and reviews is:

- raw input streams
- source coordinate data
- lexer
- spelling storage
- AST data model
- parser
- semantic compiler
- HLIR
- LLIR
- bytecode
- VM
- executable wrapper

If that structure changes, this page and the overview/module pages should change in the same PR.
