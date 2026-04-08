# PR review guide

This page is for maintainers who need to review changes quickly without re-deriving the whole compiler.

## Review order

Use this order for most PRs:

1. Identify which module owns the behavior being changed.
2. Open the matching module page and recall its invariants.
3. Check whether tests exist at the same layer.
4. Check whether the wiki should have changed.

## Which page to open

- Lexer changes: [`basedlex`](./basedlex.md)
- AST changes: [`basedast`](./basedast.md)
- Parser changes: [`basedparse`](./basedparse.md)
- Semantic, lowering, or interpreter changes: [`basedhlir`](./basedhlir.md)
- CLI wiring changes: [`based`](./based.md)
- Cross-cutting changes: [Architecture and pipeline overview](./overview.md)

## Module-specific review heuristics

### `basedlex`

Look for:

- token boundary correctness
- keyword and operator recognition consistency
- source-location correctness
- lookahead/buffering correctness

### `basedast`

Look for:

- unnecessary pointer indirections
- implementation of `span_of` for any new AST node


### `basedparse`

Look for:

- parser correctness
- precedence and associativity consistency
- edge cases which could parse in an unexpected way

### `basedhlir`

Look for:

- identifier resolution correctness
- type correctness
- mutability correctness
- constant-evaluation behavior
- correct HLIR emission
- diagnostic quality
- interpreter consistency if executable semantics changed

### `based`

Look for:

- thin orchestration only
- correct pipeline wiring
- argument parsing consistent with current executable expectations
- no compiler logic creeping into the executable

## Key invariants to keep in your head

- the interpreter may assume HLIR is valid except for explicit safety mechanisms like fuel exhaustion

## What to ask for in a PR

- tests at the changed abstraction layer
- module-page updates when the key abstraction or algorithm changed
- a short explanation when multiple layers changed
- clear justification for any new cross-layer dependency

## Wiki expectation

Every PR should update the wiki when it changes any of the following:

- the architectural split between modules
- review heuristics
- contributor workflow
- maintainer expectations around invariants

If a PR changes how a reviewer should reason about correctness, it is not fully documented until the wiki reflects that change.
