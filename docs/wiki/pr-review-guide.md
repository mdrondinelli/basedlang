# PR review guide

This page is for maintainers who need to review changes quickly without re-deriving the whole compiler.

## Review order

Use this order for most PRs:

1. Identify the layer being changed.
2. Identify the invariant that layer is supposed to preserve.
3. Check whether tests exist at the same layer.
4. Check whether the change leaked responsibility into the wrong module.
5. Check whether the wiki should have changed.

## Layer-specific review heuristics

### Lexer PRs

Look for:

- token boundary correctness
- keyword and operator recognition consistency
- source-location correctness
- lookahead/buffering correctness

Common reviewer question:

"Did this change alter the token stream in a way the parser is actually prepared to consume?"

### Parser PRs

Look for:

- AST shape correctness
- precedence and associativity consistency
- statement-versus-expression boundary correctness
- retained syntax needed for diagnostics

Common reviewer question:

"Did syntax handling change without the semantic compiler being updated for the new AST shape?"

### Semantic compiler / HLIR PRs

Look for:

- identifier resolution correctness
- type correctness
- mutability correctness
- constant-evaluation behavior
- correct HLIR emission
- diagnostic quality

Common reviewer question:

"Is this actually a semantic change, or is the compiler compensating for a bug that belongs in parsing?"

### Interpreter PRs

Look for:

- instruction semantics matching the compiler's assumptions
- correct branch and block-argument behavior
- argument/result handling
- fuel accounting

Common reviewer question:

"Did the interpreter grow logic that should instead be represented explicitly in HLIR?"

## Key invariants to keep in your head

### Name resolution invariant

Identifiers resolve through `Symbol_table` to either:

- a compile-time value
- a runtime object binding

If that distinction gets blurred, later logic becomes fragile fast.

### Type identity invariant

Semantic equality of types should be represented by canonical `Type *` identity from `Type_pool`.

### HLIR validity invariant

The compiler should emit valid HLIR; the interpreter should not have to defend against structurally impossible states except for obvious safety guards like fuel exhaustion.

### Diagnostics invariant

User-facing errors should still point to the syntax the user thinks they wrote, not a distant derived artifact.

## What to ask for in a PR

- tests at the changed abstraction layer
- doc updates when maintainers need a new mental model
- a short explanation when multiple layers changed
- clear justification for any new cross-layer dependency

## Wiki expectation

Every PR should update the wiki when it changes any of the following:

- the architectural split between modules
- the meaning of a key class or struct
- review heuristics
- contributor workflow
- maintainer expectations around invariants

If a PR changes how a reviewer should reason about correctness, it is not fully documented until the wiki reflects that change.

