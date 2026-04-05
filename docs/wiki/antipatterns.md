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

**Why it is dangerous:** `std::unreachable()` is an optimizer hint. It does not communicate the invariant to readers, and in debug builds it silently produces undefined behavior rather than a diagnostic.

**Better implementation:** Use `assert(condition)` to express internal invariants. It clearly communicates "this cannot be false here" and fires in debug builds when violated.

## Re-walking the AST to recover a type that was already compiled

**Smell:** Inside `compile_expression`, after compiling a sub-expression into an `Operand`, the code calls `type_of_expression` on the same sub-expression to recover its type.

**Why it is dangerous:** It re-walks the AST unnecessarily and can diverge from what was actually compiled if any side-effectful state changed in between.

**Better implementation:** Derive the type from the compiled `Operand` directly: call `type_of_register(r)` for a `Register` result and `type_of_constant(cv)` for a `Constant_value` result. Only call `type_of_expression` on sub-expressions that have not yet been compiled.

## Ad hoc type construction outside `Type_pool`

**Smell:** A `Type` object is constructed directly rather than going through `Type_pool`.

**Why it is dangerous:** Type identity in this compiler is pointer identity. Two `Type` objects with identical structure but different addresses compare unequal. Any type check or deduplication that relies on pointer equality silently breaks.

**Better implementation:** All type construction must go through `Type_pool`. The pool interns types so equal types share the same `Type *`.

## Diagnostics that point at derived nodes instead of user syntax

**Smell:** A diagnostic's source span points at a computed or internal AST node rather than at the token or surface syntax the user actually wrote.

**Why it is dangerous:** The user sees an error highlighted at the wrong location, which makes the error message confusing or unactionable.

**Better implementation:** Propagate source spans from the original `Lexeme` through the AST. When emitting a diagnostic, use the span of the closest enclosing user-written construct.

## Tests that only cover the happy path

**Smell:** A new feature lands with tests that only check the success case.

**Why it is dangerous:** Error paths, type mismatches, and boundary conditions are where bugs live. Happy-path-only tests give false confidence.

**Better implementation:** Add at least one test that verifies the expected diagnostic or failure behavior for invalid inputs at the same abstraction layer.

## PRs that change reviewer mental models without updating the wiki

**Smell:** A PR changes a key abstraction, algorithm, invariant, or layer boundary, but no wiki page was updated.

**Why it is dangerous:** The next reviewer derives a stale mental model from the wiki and misses the significance of the change.

**Better implementation:** See [Wiki maintenance](./wiki-maintenance.md). If the change affects how a reviewer should reason about correctness, the PR is not fully documented until the wiki reflects it.
