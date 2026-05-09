# `hlir2llir`

`hlir2llir` lowers executable HLIR into LLIR.

This is intentionally a first-pass lowering layer. It maps HLIR registers to
LLIR virtual registers, keeps block arguments as LLIR block parameters, and
preserves structured control flow so later bytecode emission can choose concrete
VM registers.

## Interface

The public surface is:

- `benson::hlir2llir::lower(hlir::Translation_unit const&)`

The result is an `llir::Translation_unit`.

## Current lowering shape

- scalar HLIR types lower to arbitrary-width LLIR bit registers
- compile-time scalar constants lower to LLIR immediates
- arithmetic, comparisons, negation, calls, jumps, branches, and returns lower
  directly to matching LLIR instructions or terminators
- void values lower to absent LLIR values

## What to keep stable

- `hlir2llir` may depend on `hlir` and `llir`
- `hlir`, `llir`, bytecode, and VM should not depend on `hlir2llir`
- this pass should remain replaceable while LLIR and bytecode lowering mature
