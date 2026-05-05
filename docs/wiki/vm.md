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

The VM loads a bytecode `Module`, initializes machine state, and executes named
functions through `Virtual_machine::call`. Registers are machine-level storage,
not HLIR registers. Calls slide a register window over a dynamic register array.
Pointer values identify an address space plus an offset.

The stack pointer is VM-owned state, not a bytecode register. Bytecode can push
stack space for addressable locals, and the VM restores the caller stack pointer
when a function returns.

Wide bytecode instructions are dispatched through the same execution model as
their narrow forms, but decode 16-bit register operands and wider
immediate/constant operands.

## What to keep stable

- VM owns execution of bytecode, not bytecode encoding
- bytecode tests should check encoding; VM tests should check behavior after
  execution
- keep the VM independent from AST, parsing, frontend lowering, and HLIR details
- avoid documenting instruction-by-instruction semantics here while the opcode
  set is still changing
