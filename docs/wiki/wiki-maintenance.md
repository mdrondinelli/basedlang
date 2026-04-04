# Wiki maintenance

The wiki exists to keep maintainer context from decaying.

## Rule

Every PR should update the wiki when it changes how a maintainer should understand, review, or extend the system.

## What must trigger a wiki update

- a new major abstraction
- a changed responsibility boundary between modules
- a changed invariant
- a changed review heuristic
- a changed contributor workflow
- a new language feature that affects implementation mental models

## What usually does not need a wiki update

- mechanical refactors that do not change responsibilities
- local renames that do not change concepts
- test-only changes with no change in maintainer understanding

## Preferred maintenance style

- document concepts, not filenames
- document invariants, not transient implementation details
- document why a layer owns behavior, not just where behavior currently lives
- keep navigation obvious enough that a reviewer can jump directly to the right page

## When pages should be split

Split a page when one of these becomes true:

- reviewers cannot quickly find a concept
- one page mixes architecture with step-by-step workflow
- a section keeps growing because it represents a distinct stable topic

## Relationship to other docs

- [`language.md`](../../language.md) is the language-surface reference
- the wiki is the maintainer and reviewer reference

If the language changes, `language.md` and the relevant wiki pages should usually move together.

