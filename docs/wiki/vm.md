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

A `Virtual_machine` owns its execution state directly: an instruction
pointer into the module's code, a stack pointer into a fixed-size linear
stack buffer, a register vector, a `base_register` cursor naming the start
of the currently executing function's register window, and a vector of
`Call_frame` records that is the call stack. Pointer values in registers
identify an address space plus an offset.

Functions execute against a sliding register window. The bytecode-level
register `gpr(n)` resolves to absolute register `base_register + n`, and a
function may touch slots `0` through `register_count - 1` relative to that
base. Each `call_i` / `call_void_i` slides the window forward by an amount
the caller picks via the `base` operand. Arguments are passed by writing
them into the registers that will become `gpr(0)..gpr(args - 1)` of the
callee's window before the call instruction executes; the callee just sees
them as its own low registers.

Register storage is not pre-allocated. The vector starts empty and grows
as call windows demand more slots, through an internal helper that doubles
capacity when needed so the amortized cost of growth stays constant. The
helper is independent of `std::vector::resize`'s growth behavior, which the
standard does not require to amortize.

Every `call_i` and `call_void_i` pushes a `Call_frame` capturing the
caller's return address, `base_register`, `stack_pointer`, and an optional
return-register slot. The return slot is an absolute register index into
the caller's frame and is present only for value-returning calls. `ret`
reads the returned value from the callee's frame, restores the caller's
base register and stack pointer, and writes the value through the
absolute return-register index recorded in the popped frame. `ret_void`
performs the same restoration but writes nothing. Both pop the frame.

The host enters bytecode through `Virtual_machine::call(name, args)`. It
resolves the name through the module's `function_indices`, validates the
argument count and the type of each argument against the function's
parameter types, and then sets up an absolute base-zero window. Arguments
are copied into `gpr(0)..gpr(args.size() - 1)` of that window, marshaled
according to their `Scalar` type. When the function's return type is
non-void, an extra slot is allocated immediately past the function's
register window to receive the return value. A synthetic `Call_frame` is
pushed whose return address points at a local exit byte, so that when the
top-level callee returns, the dispatch loop's next opcode is `exit` and
`run` exits cleanly. The return value is then decoded from the per-call
return slot.

Exception safety on host entry is provided by an internal `State_guard`
RAII type. Before transferring control to bytecode, `call` snapshots the
instruction pointer, base register, stack pointer, and call-stack size,
and a `State_guard` restores them on scope exit whether or not bytecode
threw. Register storage may have grown during the call, but the caller's
window is unaffected because `base_register` is restored to its prior
value, and unused slots above the caller's window are simply latent
capacity for the next call.

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
