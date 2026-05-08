# `vm`

`vm` executes bytecode modules.

This layer is still experimental. It exists to run the current low-level
bytecode representation and should not be read as the final execution strategy
for the language.

## Interface

The public surface is centered on:

- `Virtual_machine`
- `Call_frame` and the call stack
- register storage
- bytecode module loading
- bytecode dispatch
- stack and constant address spaces
- `Virtual_machine::call(name, args)` host entry

These live under the `benson` namespace, with bytecode inputs coming from
`benson::bytecode`.

## Core model

A `Virtual_machine` owns the mutable execution state needed to run a
bytecode module: instruction position, stack position, register storage,
the current register-window base, and the call stack. Pointer values in
registers identify an address space plus an offset.

Functions execute against a register window. Bytecode registers are
function-relative, and the VM maps them onto backing register storage
through the current window base. Calls create a callee window, returns
restore the caller's window and stack position, and value-returning calls
copy the returned value back into the caller's requested destination.

The host enters bytecode through `Virtual_machine::call(name, args)`. It
resolves the function name, validates the argument count and scalar types,
marshals arguments into the callee's low registers, runs the function, and
decodes the return value when one exists. If bytecode execution throws, the
host-visible VM execution state is restored before the exception leaves
`call`.

Wide bytecode instructions are dispatched through the same execution model
as their narrow forms, but decode 16-bit register operands and wider
immediate and constant operands.

## What to keep stable

- VM owns execution of bytecode, not bytecode encoding
- bytecode tests should check encoding; VM tests should check behavior after
  execution
- keep the VM independent from AST, parsing, frontend lowering, and HLIR details
- avoid documenting instruction-by-instruction semantics here while the opcode
  set is still changing
