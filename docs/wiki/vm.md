# `vm`

`vm` executes bytecode modules.

This layer is still experimental. It exists to run the current low-level
bytecode representation and should not be read as the final execution strategy
for the language.

## Interface

The public surface is centered on:

- `Virtual_machine`
- register storage
- bytecode module loading
- bytecode dispatch
- stack and constant address spaces

These live under the `benson` namespace, with bytecode inputs coming from
`benson::bytecode`.

## Core model

The VM loads a bytecode `Module`, initializes machine state, and dispatches
instructions until the program exits. Registers are machine-level storage, not
HLIR registers. Pointer values identify an address space plus an offset.

Wide bytecode instructions are dispatched through the same execution model as
their narrow forms, but decode wider operands.

## What to keep stable

- VM owns execution of bytecode, not bytecode encoding
- bytecode tests should check encoding; VM tests should check behavior after
  execution
- keep the VM independent from AST, parsing, frontend lowering, and HLIR details
- avoid documenting instruction-by-instruction semantics here while the opcode
  set is still changing
