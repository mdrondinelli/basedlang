# `bytecode`

`bytecode` is the low-level executable instruction format used by the current
VM work.

This layer is still young and should be treated as implementation infrastructure,
not as a stable file format or language-level contract.

## Interface

The public surface is centered on:

- registers
- opcodes
- immediates
- constants
- `Module`
- `Bytecode_writer`
- `Module_builder`

All of these live in the `benson::bytecode` namespace.

## Core model

A `Module` carries the encoded instruction bytes alongside the data needed to
execute and reference into them. That data is a constant blob, a constant
table indexing into the blob, a vector of `Function` records, and a name-to-
index map keyed by `Spelling`. A `Function` records its entry position in the
code stream, its parameter and return types, and the number of register slots
its body may access. The map's values are indices into the function vector;
those indices are stable for the lifetime of the module and are what the
`call_i` and `call_void_i` opcodes consume as operands.

Functions are produced through `Module_builder` in two steps. `declare_function`
takes a name, parameter types, return type, and register count, reserves the
next index in the function vector, and returns it as an `Immediate`. The body
is then emitted through the writer, and `place_function` stamps the writer's
current position into the function record. The two-step shape is what makes
forward references work: the index exists from the moment a function is
declared, so other code can emit calls to it before its body has been placed.
`build` rejects any function that was declared but never placed.

The `register_count` declared on a function bounds how many register slots
its body may reference relative to its base. A function declared with
`register_count == 0` may not access any register at runtime. This is a
programmer-level contract enforced at the bytecode-emitter level, not at
runtime.

Calls go through function indices. `call_i fn, base, dst` is the value-
returning form: `fn` is a function-index immediate, `base` names the start
of the callee's register window expressed in the caller's frame, and `dst`
is the caller-relative register that will receive the return value. The
void variant `call_void_i fn, base` drops `dst`. Returns are `ret src`,
which writes the value held in the caller-relative-from-the-callee
register `src` back into the caller's `dst`, and `ret_void`, which writes
nothing. Both restore the caller's stack pointer.

The stack pointer is VM-owned and is not addressable as a bytecode register.
Bytecode reserves stack-local storage with `alloca_i amount`, taking an
immediate byte count, or `alloca reg`, taking a register that holds a
non-negative byte count. Stack memory is then accessed through `mov_sp_i
dst, offset`, which materializes a stack-space `Pointer` at `sp + offset`
into a register, or directly through `load_sp_N dst, offset` and
`store_sp_N src, offset` for `N` in {1, 2, 4, 8}. For `N < 8` the load
sign-extends into a 64-bit register; for `N == 8` it is a plain 64-bit
copy.

General registers are zero-based: `gpr(n)` is valid for any `n >= 0`. There
is no longer any sentinel register reserved for the stack pointer.

Operand-taking instructions have narrow and wide wire forms. The `wide`
opcode prefixes an instruction when any register, immediate, or constant
operand needs the larger encoding. In a wide instruction, register operands
are encoded as 16-bit values and immediates and constants take their wide
widths.

## What to keep stable

- bytecode owns the encoded instruction stream, not VM execution behavior
- writer tests should cover emitted bytes for new instruction shapes
- module-builder tests should cover label, function-placement, and
  constant-building behavior
- do not treat opcode numbers or binary compatibility as stable unless that is
  made explicit in the future
