# `ir`

`ir` is the HLIR data model and its interpreter.

It does **not** consume AST. The AST→HLIR transformation lives in
[`frontend`](./frontend.md); `ir` just defines the shape of HLIR and how
to execute it.

## Interface

The public surface is centered on:

- `Type` and `Type_pool`
- `Constant_value`
- `Register`, `Operand`, `Instruction`, `Terminator`
- `Basic_block`, `Function`, `Translation_unit`
- `interpret(...)` and `Fuel_exhausted_error`

All of these live in the `benson::ir` namespace.

## Core abstractions

### `Type` and `Type_pool`

The canonical type system representation and the interning pool that makes
pointer identity meaningful.

### `Constant_value`

The compile-time value domain, including 8-, 16-, 32-, and 64-bit signed
integers, booleans, `void`, type values, and function values.

### HLIR objects

The executable model is:

- `Register`
- `Operand`
- `Instruction`
- `Basic_block`
- `Terminator`
- `Function`
- HLIR `Translation_unit`

`Operand` is the value form used during lowering. It can represent either a
compile-time constant or a runtime register.

Key properties of these types:

- `Register` is an opaque integer ID. A default-constructed `Register` is invalid.
- `Operand` is `std::variant<Register, Constant_value>`.
- `Basic_block` has parameters (registers), a list of instructions, and a `Terminator`.
- `Function` owns its blocks and a monotonically increasing register counter.

## Interpretation

The interpreter executes HLIR by:

1. allocating register storage
2. seeding entry-block parameters
3. executing instructions in order
4. following block terminators
5. returning a `Constant_value`

It uses fuel accounting to stop runaway compile-time execution.

## What to keep stable

- type identity must come from `Type_pool`
- executable HLIR should stay self-contained
- the interpreter may assume HLIR is valid except for explicit safety mechanisms like fuel exhaustion
- `ir` must not gain a dependency on `ast` or on the frontend
