# bensonlang Wiki

This wiki is the maintainer-facing map of the project.
Use it to find the right abstraction, workflow, or review context quickly.
It is organized for jumping, not for linear reading.

## Start here

- [Architecture and pipeline overview](./overview.md)
- [Project structure](./project-structure.md)
- [Tutorial: first change](./tutorial-first-change.md)

## Common tasks

### I need to make a change

Read these first:

- [Tutorial: first change](./tutorial-first-change.md)
- [Conventions](./conventions.md)
- [PR review guide](./pr-review-guide.md)
- [Architecture and pipeline overview](./overview.md)
- [Project structure](./project-structure.md)

Review [Conventions](./conventions.md) before drafting or opening the PR, even
for small changes. Then jump to the module page for the area you are editing.

### I need to review a PR

Read these first:

- [PR review guide](./pr-review-guide.md)
- [Architecture and pipeline overview](./overview.md)
- the module page that owns the changed behavior

Use the review guide as the main workflow page. Use module pages for the
invariants and concepts that the PR is changing.

### I need to find the owning module

Start with [Project structure](./project-structure.md), then confirm the layer
boundary in [Architecture and pipeline overview](./overview.md).

### I need coding and process conventions

Read [Conventions](./conventions.md).

## Reference

### Architecture and language

- [Architecture and pipeline overview](./overview.md)
- [Project structure](./project-structure.md)
- [Language reference](./language.md)

### Modules

- [`source`](./source.md)
- [`streams`](./streams.md)
- [`lexing`](./lexing.md)
- [`spelling`](./spelling.md)
- [`ast`](./ast.md)
- [`parsing`](./parsing.md)
- [`ir`](./ir.md)
- [`frontend`](./frontend.md)
- [`bytecode`](./bytecode.md)
- [`vm`](./vm.md)
- [`benson`](./benson.md)

### Process

- [Tutorial: first change](./tutorial-first-change.md)
- [PR review guide](./pr-review-guide.md)
- [Conventions](./conventions.md)
- [Antipatterns for reviewers](./antipatterns.md)
