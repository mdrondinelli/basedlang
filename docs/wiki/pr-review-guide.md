# PR review guide

This page is for maintainers who need to review changes quickly without re-deriving the whole compiler.

## What a reviewer is trying to establish

A review should answer these questions:

1. Does the changed code live in the correct layer?
2. Does the implementation preserve that layer's invariants?
3. Are cross-layer effects explicit and justified?
4. Do tests exist at the abstraction level that actually changed?
5. Does the wiki still match how a maintainer should reason about the system?

## Review sequence

Use this order for most PRs:

1. Identify the owning layer.
2. Read the matching module page before judging implementation details.
3. Check whether the change stayed inside that layer or crossed boundaries.
4. Check whether tests match the changed abstraction.
5. Check whether the wiki should have changed in the same PR.

## Find the owning layer

Use this map first:

- tokenization and token lookahead: [`lexing`](./lexing.md)
- AST node shapes and source spans over AST: [`ast`](./ast.md)
- syntax consumption and AST construction: [`parsing`](./parsing.md)
- semantic lowering, diagnostics, type evaluation, and symbol resolution: [`frontend`](./frontend.md)
- HLIR data model and interpretation: [`ir`](./ir.md)
- bytecode encoding and module construction: [`bytecode`](./bytecode.md)
- bytecode execution: [`vm`](./vm.md)
- executable orchestration and CLI behavior: [`benson`](./benson.md)
- cross-cutting or unclear ownership: [Architecture and pipeline overview](./overview.md)

If ownership is unclear after this step, treat that as a review concern rather
than reviewing the change as though the boundary does not matter.

## Check the changed abstraction

Open the module page that owns the change and review against that page's
concepts and invariants. When the change depends on project-wide coding or
error-handling policy, also check [Conventions](./conventions.md).

Do not start from line-by-line implementation details. First establish what the
module is supposed to own, then ask whether the code matches that model.

When the PR changes behavior in one layer but the explanation depends on
another, confirm that the dependency direction is still sensible.

## Check cross-layer fallout

Cross-layer changes are higher-risk than local ones.

Ask for an explicit explanation when a PR changes multiple layers. A good PR
should make it clear why that was necessary instead of forcing the reviewer to
infer it.

Look for:

- the minimal set of affected layers
- a clear ownership reason for each changed layer
- tests at each affected abstraction where behavior changed
- no new dependency that collapses an intended module boundary

## Check tests

Tests should follow the changed abstraction, not just the easiest harness.

Use this rule of thumb:

- lexer behavior change: lexer tests
- parser behavior change: parser tests
- semantic or lowering change: frontend or IR-facing tests
- bytecode encoding change: bytecode tests
- bytecode execution change: VM tests
- executable wiring change: CLI-facing tests

Be skeptical of PRs that only add happy-path tests or only test a downstream
layer when the real behavior change happened upstream.

## Check docs and wiki

The wiki should change in the same PR when the change affects how a maintainer
should understand, review, or extend the system.

That includes:

- a new major abstraction
- a changed responsibility boundary between modules
- a changed invariant
- a changed contributor workflow
- a changed reviewer expectation
- a language feature that changes implementation mental models

This usually does not include:

- mechanical refactors that do not change responsibilities
- local renames that do not change concepts
- test-only changes with no change in maintainer understanding

Document concepts, not filenames, and keep the navigation obvious enough that a
later reviewer can jump directly to the right page.

## What should block approval

Escalate or block when:

- the owning layer is unclear
- the implementation changes multiple layers without a clear reason
- tests do not match the changed abstraction
- a new cross-layer dependency is not justified
- the wiki is stale relative to the reviewer mental model required by the PR
- the change silently moves responsibility from one module to another

## Supporting page

Use [Antipatterns for reviewers](./antipatterns.md) as a supporting catalog of
common review smells, not as the main review workflow.
