# `frontend`

`frontend` is the semantic core of the compiler. It consumes AST and
produces executable HLIR (the [`ir`](./ir.md) data model).

## Interface

The public surface is centered on:

- `compile(...)`
- `Frontend`
- `Diagnostic`, `Diagnostic_note`, `Compilation_failure`
- `Symbol`, `Symbol_table`, `Object_binding`
- `Unary_operator_overload`, `Binary_operator_overload`

All of these live in the top-level `benson::` namespace, alongside
`benson::Parser` and `benson::Lexeme_stream`. They reference HLIR types
through the `benson::ir::` qualifier.

## Core abstractions

### `Symbol_table`

Maps names to semantic bindings across global and nested scopes.

### `Symbol`

A resolved name that holds either:

- an `Object_binding` (runtime: type + register + mutability)
- an `ir::Constant_value` (compile-time value)

## Data model

The key semantic data model is:

1. AST nodes still carry source identity
2. identifiers resolve to either runtime objects or compile-time values
3. types are canonicalized through `ir::Type_pool`
4. valid programs lower into HLIR made of blocks, registers, instructions, and terminators

## Algorithms

### Semantic compilation

`Frontend` walks the AST and handles:

- builtin seeding
- name resolution
- type checking
- constant evaluation
- operator application
- diagnostics
- HLIR emission

`compile_expression` returns an `ir::Operand` — either a `Register`
(runtime result) or an `ir::Constant_value` (compile-time result).
Constant folding happens naturally: if all inputs to an operation are
`Constant_value`, the result is evaluated at compile time without
emitting any instructions.

Register types are tracked in a table indexed by register ID. This table
is saved and restored per function so nested function compilations do not
interfere with each other.

Symbol bindings hold either a runtime `Object_binding` (type + register +
mutability) or a compile-time `ir::Constant_value`.

## What to keep stable

- type identity must come from `ir::Type_pool`
- diagnostics should point at user-facing syntax
- HLIR emitted by the frontend should remain self-contained (no
  references back into AST, parser state, etc.)
