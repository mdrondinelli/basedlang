# `basedhlir`

`basedhlir` is the semantic core of the project.

## Interface

The public surface is centered on:

- `compile(...)`
- `interpret(...)`
- `Compilation_context`
- `Symbol_table`
- `Type` and `Type_pool`
- `Constant_value`
- `Function`, `Basic_block`, `Instruction`, and `Terminator`

This module consumes AST and produces executable HLIR.

## Core abstractions

### `Symbol_table`

Maps names to semantic bindings across global and nested scopes.

### `Symbol`

A resolved name that holds either:

- an `Object_binding`
- a `Constant_value`

That split is the semantic center of gravity of the compiler.

### `Type` and `Type_pool`

The canonical type system representation and the interning pool that makes pointer identity meaningful.

### `Constant_value`

The compile-time value domain, including integers, booleans, `void`, type values, and function values.

### HLIR objects

The executable model is:

- `Register`
- `Operand`
- `Instruction`
- `Basic_block`
- `Terminator`
- `Function`
- HLIR `Translation_unit`

## Data model

The key semantic data model is:

1. AST nodes still carry source identity
2. identifiers resolve to either runtime objects or compile-time values
3. types are canonicalized through `Type_pool`
4. valid programs lower into HLIR made of blocks, registers, instructions, and terminators

That is the shortest useful way to think about the whole module.

## Algorithms

There are three main kinds of work here.

### Semantic compilation

`Compilation_context` walks the AST and handles:

- builtin seeding
- name resolution
- type checking
- constant evaluation
- diagnostics
- HLIR emission

### Type canonicalization

`Type_pool` interns types so equal types share the same `Type *`.

### Interpretation

The interpreter executes HLIR by:

1. allocating register storage
2. seeding entry-block parameters
3. executing instructions in order
4. following block terminators
5. returning a `Constant_value`

It uses fuel accounting to stop runaway compile-time execution.

## What to keep stable

- resolved names must stay clearly split between object bindings and constant values
- type identity must come from `Type_pool`
- diagnostics should point at user-facing syntax
- the interpreter should execute valid HLIR, not compensate for invalid HLIR

