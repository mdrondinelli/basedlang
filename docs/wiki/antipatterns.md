# Antipatterns for reviewers

When you add a new antipattern:

1. describe the smell
2. explain why it is dangerous in this codebase specifically
3. describe what a better implementation usually looks like
4. mention what tests or docs should accompany the fix

## Using `emit_error` for unimplemented features

**Smell:** `emit_error(...)` is called for a code path that simply has not been implemented yet.

**Why it is dangerous:** `emit_error` produces a user-facing diagnostic. When the compiler emits it for an unimplemented path, the user sees a misleading type or syntax error instead of an honest "not yet supported" crash. It also hides the gap from future implementers.

**Better implementation:** Use `throw std::runtime_error(...)` for unimplemented features. Reserve `emit_error` for genuine user-input errors (type mismatches, undefined identifiers, etc.). Use `std::unreachable()` only for paths that are structurally impossible.

## Using `std::unreachable()` to express invariants

**Smell:** An `else` or `default` branch contains `std::unreachable()` to paper over a condition that must always be true.

**Why it is dangerous:** `std::unreachable()` is an optimizer hint. It does not communicate the invariant to readers, and it produces undefined behavior rather than a diagnostic.

**Better implementation:** Use `assert(condition)` to express internal invariants. It clearly communicates "this cannot be false here" and fires in debug builds when violated.

## Using `auto const*` or `auto*` for local pointer declarations

**Smell:** A local pointer declaration is written as `auto const* x = ...`, `auto* x = ...`, or similar.

**Why it is a problem here:** This is a project convention issue, not a semantic bug by itself. In this codebase we want `auto` to absorb the pointer type so declarations read consistently and constness is easier to scan.

**Better implementation:** Use `auto const x = ...` for const-pointer results and `auto x = ...` for mutable-pointer results. See [Conventions](./conventions.md).

## Tests that only cover the happy path

**Smell:** A new feature lands with tests that only check the success case.

**Why it is dangerous:** Error paths, type mismatches, and boundary conditions are where bugs live. Happy-path-only tests give false confidence.

**Better implementation:** Add at least one test that verifies the expected diagnostic or failure behavior for invalid inputs at the same abstraction layer.

## PRs that change reviewer mental models without updating the wiki

**Smell:** A PR changes a key abstraction, algorithm, invariant, or layer boundary, but no wiki page was updated.

**Why it is dangerous:** The next reviewer derives a stale mental model from the wiki and misses the significance of the change.

**Better implementation:** See [PR review guide](./pr-review-guide.md). If the change affects how a reviewer should reason about correctness, the PR is not fully documented until the wiki reflects it.
