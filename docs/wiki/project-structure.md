# Project structure

This page describes the repository in stable conceptual terms. It intentionally avoids depending on individual source filenames.

## Main code areas

### `code/basedlex`

The lexer library. Owns character input, buffering/lookahead, tokenization, and source-location tracking at the token level.

See [`basedlex`](./basedlex.md).

### `code/basedparse`

The parser library. Owns AST construction, expression precedence, statement parsing, and source-span recovery over the AST.

See [`basedparse`](./basedparse.md).

### `code/basedhlir`

The semantic and execution core. Owns:

- types
- symbol resolution
- diagnostics
- compile-time values
- lowering to HLIR
- interpretation of HLIR

See [`basedhlir`](./basedhlir.md).

### `code/based`

The CLI entrypoint.

See [`based`](./based.md).

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
2. Is this about syntax shape only?
3. Is this about meaning, type checking, lowering, or diagnostics?
4. Is this only about wiring the executable?

Those map directly to the four main modules.

## Stability guidelines

The repo may be reorganized internally over time. The stable structure to preserve in docs and reviews is:

- lexer
- parser
- semantic compiler / HLIR
- executable wrapper

If that structure changes, this page and the overview/module pages should change in the same PR.
