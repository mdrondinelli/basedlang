# basedlang Wiki

This wiki is the maintainer-facing map of the project.

It is written for two use cases:

1. Someone needs to make a change and wants the shortest path to the right abstraction.
2. Someone needs to review a PR and wants to understand whether the change is correct at the architectural level.

The pages below are organized to support fast navigation rather than linear reading.

## Start here

- [Tutorial: first change](./tutorial-first-change.md)
- [Language reference](./language.md)
- [Architecture and pipeline overview](./overview.md)
- [`basedlex`](./basedlex.md)
- [`basedast`](./basedast.md)
- [`basedparse`](./basedparse.md)
- [`basedhlir`](./basedhlir.md)
- [`based`](./based.md)
- [Project structure](./project-structure.md)
- [Conventions](./conventions.md)
- [Formatting](./formatting.md)
- [PR review guide](./pr-review-guide.md)
- [Antipatterns for reviewers](./antipatterns.md)
- [Wiki maintenance](./wiki-maintenance.md)

## Fast paths

### I need to review a PR

Read these first:

- [PR review guide](./pr-review-guide.md)
- [Architecture and pipeline overview](./overview.md)
- [`basedhlir`](./basedhlir.md)

Then jump to the module page for the code being changed.

### I need to draft a PR

Read these first:

- [Tutorial: first change](./tutorial-first-change.md)
- [Architecture and pipeline overview](./overview.md)
- [Project structure](./project-structure.md)
- [Formatting](./formatting.md)
- [Conventions](./conventions.md)

Then jump to the module page for the area you are editing.

### I need to understand how a concept flows end to end

Use this sequence:

1. [Architecture and pipeline overview](./overview.md)
2. [`basedlex`](./basedlex.md), [`basedparse`](./basedparse.md), [`basedhlir`](./basedhlir.md), or [`based`](./based.md)
