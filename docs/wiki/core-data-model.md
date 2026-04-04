# Core data model

This page is the shortest path to understanding the key classes and structs that shape the compiler.

## Lexing data

### `Token`

The closed set of token kinds recognized by the lexer. This is the shared vocabulary between lexing and parsing.

### `Lexeme`

A token plus the original source text and source location.

Why it matters:

- diagnostics depend on it
- AST nodes retain it heavily
- parser behavior is mostly driven by lexeme kind, but semantic code often needs the text too

## AST data

### `Expression`

A `std::variant` over all expression node types. This is the main syntax sum type.

The important mental model is that expression variants fall into three groups:

- value-producing source forms such as literals, identifiers, blocks, and `if`
- operator-structured forms such as prefix, postfix, and binary expressions
- forms that produce higher-order semantic entities such as functions and type expressions

### `Statement`

A `std::variant` over top-level executable statement forms inside blocks.

### `Translation_unit`

Currently a sequence of top-level `let` statements. This is a simple shape, but it is semantically important because top-level bindings define the root environment the compiler sees.

## Semantic data

### `Constant_value`

The compile-time value domain. It currently includes:

- `Int32`
- `Bool`
- `Void`
- type values
- function values

This is one of the most important concepts in the project. If an expression can be reduced during compilation, it becomes a `Constant_value`.

### `Symbol`

A named binding in the semantic environment.

It can represent either:

- an `Object_binding`, meaning a typed runtime object living in a register
- a `Constant_value`, meaning a compile-time-only binding

That split is central to understanding identifier resolution.

### `Object_binding`

Represents a mutable or immutable runtime object, its type, and the register that currently stores it.

### `Symbol_table`

The layered environment for name resolution.

Important properties:

- it has a global scope and nested local scopes
- scopes can be barriers
- lookup walks outward until it hits a barrier, then falls back to globals

Barrier scopes are an important semantic tool because they limit what inner contexts are allowed to capture or observe.

### `Type`

The canonical semantic representation of a type.

It is itself a tagged union over:

- primitive types
- the meta-type `Type`
- pointer types
- sized arrays
- unsized arrays
- function types

`Type` also carries caches for derived types. That means type identity is intentionally pointer-based after interning.

### `Type_pool`

Owns all type objects and interns them so semantically equal types reuse the same `Type *`.

This is the reason pointer equality on types is meaningful across the compiler.

### `Unary_operator_overload` and `Binary_operator_overload`

The semantic definition of operators. They answer:

- whether an operator applies
- what result type it produces
- how it evaluates constant operands

They are the bridge between syntax-level operators and type/value semantics.

## HLIR data

### `Register`

A lightweight identifier for a typed temporary in a function.

The register itself does not store type information. The compiler context tracks register types separately while emitting HLIR.

### `Operand`

Either a register or a constant. This allows instructions to mix folded constants with runtime temporaries.

### `Instruction`

A `std::variant` over executable HLIR operations such as constant creation, copies, unary ops, binary ops, and calls.

### `Basic_block`

A straight-line sequence of instructions ending in one terminator.

### `Terminator`

The control-flow edge for a block. No block is complete without one.

### `Function`

Compiled executable unit consisting of:

- a function type
- a list of blocks
- a register count

### HLIR `Translation_unit`

The compiled program:

- owns all functions
- exposes a function lookup table by name
- owns operator overload instances used by emitted instructions

That ownership detail matters because instructions keep raw pointers to overload objects and functions.

## Cross-cutting invariants

These are the assumptions many parts of the code rely on:

- type identity is canonicalized through `Type_pool`
- HLIR instructions reference valid registers and owned overload objects
- `Symbol_table` returns either an object binding or a compile-time value, never an ambiguous middle state
- AST nodes preserve enough lexeme information for diagnostics
- interpreter input is assumed to be semantically valid HLIR

