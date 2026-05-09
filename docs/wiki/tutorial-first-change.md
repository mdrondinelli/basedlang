# Tutorial: first change

This tutorial is for a maintainer who needs to get from zero context to a small, correct change quickly.

## Goal

Build a reliable mental model before touching code:

1. understand the layer you are changing
2. understand the key data structures in that layer
3. do the work on a dedicated branch
4. update tests and docs with the behavior change

## Step 0: Start from a branch

Before editing tracked files, create a dedicated branch for the change.
**Do this first, even for small changes.**

Example:

```sh
git switch -c short-description-of-change
```

## Step 1: Understand the product shape

Read:

1. [Architecture and pipeline overview](./overview.md)
2. [Language reference](./language.md)
3. [Project structure](./project-structure.md)

After that, you should be able to answer:

- Is my change lexical, syntactic, semantic, or executable wiring?
- Will it affect compile-time evaluation, runtime interpretation, or both?

## Step 2: Find the right abstraction

Use this shortcut:

- New token or tokenization rule: lexer
- New syntax or precedence rule: parser
- New typing rule, mutability rule, constant rule, or lowering rule: semantic compiler
- New executable behavior around invoking compiled code: CLI or interpreter

Start from "which layer owns this behavior?"

## Step 3: Load the key concepts for that layer

### If you are changing lexing

Understand:

- [`lexing`](./lexing.md)

### If you are changing parsing

Understand:

- [`parsing`](./parsing.md)

### If you are changing semantics or lowering

Understand:

- [`hlir`](./hlir.md)
- [`llir`](./llir.md)
- [`hlir2llir`](./hlir2llir.md)

### If you are changing interpretation

Understand:

- [`hlir`](./hlir.md)

### If you are changing executable wiring

Understand:

- [`benson`](./benson.md)

## Step 4: Make the smallest coherent change

Good examples:

- adding one operator and its tests
- extending one syntax form and its lowering
- improving one diagnostic path
- adding one review-oriented wiki clarification

## Step 5: Validate at the right layer

Your test strategy should match the abstraction you changed.

- Lexer change: test tokenization and locations.
- Parser change: test AST shape or parse success/failure behavior.
- Semantic change: test compilation behavior, diagnostics, and HLIR-facing semantics.
- Interpreter change: test executed results and control-flow behavior.

### Build and run tests

This repo builds Catch2 test executables with CMake.

Configure and build:

```sh
cmake -S . -B build
cmake --build build
```

Run the relevant test binary directly:

```sh
./build/streams-tests
./build/lexing-tests
./build/parsing-tests
./build/hlir-tests
./build/llir-tests
./build/hlir2llir-tests
```

Before committing, format the changed code. See [Conventions](./conventions.md).

## Step 6: Update the wiki in the same PR

The wiki is not optional process overhead. It is part of keeping reviewer context fresh.

Update it when your change affects:

- a key abstraction
- a layer boundary
- the contributor decision tree
- reviewer expectations

## Quick checklist

Before opening the PR, confirm:

- the change was done on a dedicated branch, not `main`
- [Conventions](./conventions.md) were reviewed before drafting the PR
- the changed module is the correct owner
- tests moved with behavior
- docs moved with reviewer mental model
- changed code was formatted before committing
- cross-layer fallout was handled explicitly
