# `llir`

`llir` is the low-level IR data model that will sit between HLIR and bytecode.

It is intended to be bytecode-friendly without being bytecode itself. Bytecode
still owns encoding and VM execution details; LLIR owns a structured virtual
register representation suitable for out-of-SSA lowering.

## Interface

The public surface is centered on:

- `Register`
- `Register_type`
- `Immediate` and `Operand`
- `Instruction`
- `Basic_block`
- `Function`
- `Translation_unit`

All of these live in the `benson::llir` namespace.

## Core abstractions

### Registers and types

LLIR registers are virtual and infinite for now. Each function owns a register
type table, and each allocated register indexes into that table.

The only v1 register type is `Bit_register_type`, which stores an arbitrary
positive bit width. This keeps integer and boolean values explicit while leaving
room for a future pointer register type, whose size is target-dependent.

### Blocks and control flow

Blocks carry parameters, instructions, and terminators. Terminators can jump,
branch, or return. Jump and branch terminators carry argument operands so block
arguments can model out-of-SSA data movement before bytecode emission decides
where those values live.

## What to keep stable

- LLIR should stay independent from AST, frontend, HLIR internals, bytecode
  encoding, and VM execution.
- LLIR register types should not assume pointer width.
- Bytecode lowering may be sloppy at first, but LLIR should remain structured
  enough to replace that lowering later.
