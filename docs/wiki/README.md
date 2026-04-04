# basedlang Wiki

This wiki is the maintainer-facing map of the project.

It is written for two use cases:

1. Someone needs to make a change and wants the shortest path to the right abstraction.
2. Someone needs to review a PR and wants to understand whether the change is correct at the architectural level.

The pages below are organized to support fast navigation rather than linear reading.

## Start here

- [Tutorial: first change](./tutorial-first-change.md)
- [Project structure](./project-structure.md)
- [Architecture overview](./architecture-overview.md)
- [Compiler pipeline](./compiler-pipeline.md)
- [Core data model](./core-data-model.md)
- [PR review guide](./pr-review-guide.md)
- [Antipatterns for reviewers](./antipatterns.md)
- [Wiki maintenance](./wiki-maintenance.md)

## Fast paths

### I need to review a PR

Read these first:

- [PR review guide](./pr-review-guide.md)
- [Architecture overview](./architecture-overview.md)
- [Core data model](./core-data-model.md)

Then use the module-specific sections in [Compiler pipeline](./compiler-pipeline.md) to sanity-check the layer being changed.

### I need to make a change

Read these first:

- [Tutorial: first change](./tutorial-first-change.md)
- [Project structure](./project-structure.md)
- [Architecture overview](./architecture-overview.md)

Then jump to the relevant module section in [Compiler pipeline](./compiler-pipeline.md).

### I need to understand how a concept flows end to end

Use this sequence:

1. [Architecture overview](./architecture-overview.md)
2. [Compiler pipeline](./compiler-pipeline.md)
3. [Core data model](./core-data-model.md)

## External source of truth

The language surface area is described in [`language.md`](../../language.md). This wiki focuses on implementation and review, not language syntax duplication.

