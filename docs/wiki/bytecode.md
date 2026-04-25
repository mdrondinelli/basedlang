# `bytecode`

`bytecode` is the low-level executable instruction format used by the current
VM work.

This layer is still young and should be treated as implementation infrastructure,
not as a stable file format or language-level contract.

## Interface

The public surface is centered on:

- registers
- opcodes
- narrow and wide immediates
- constants
- `Module`
- `Bytecode_writer`
- `Module_builder`

All of these live in the `benson::bytecode` namespace.

## Core model

A bytecode `Module` contains instruction bytes plus constant storage. The
writer emits compact instruction encodings. `Module_builder` adds conveniences
around labels and constants while still producing the same module shape.

Some operands have narrow and wide forms. The `wide` opcode prefixes an
instruction when the following operand needs the larger encoding.

## What to keep stable

- bytecode owns the encoded instruction stream, not VM execution behavior
- writer tests should cover emitted bytes for new instruction shapes
- module-builder tests should cover label and constant-building behavior
- do not treat opcode numbers or binary compatibility as stable unless that is
  made explicit in the future
