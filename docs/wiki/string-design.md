# Compiler String Design Proposal

This document proposes how the compiler should represent source text, lexeme
text, and semantic names as the language grows beyond its current ASCII-heavy
surface.

It is written to make one concrete recommendation, not to leave the design
open-ended.

## Summary

Adopt a split text model:

- keep the original source file bytes as the source of truth for fidelity
- make spans point into that source directly, without deriving end positions
  from stored token byte length
- introduce a project-owned text type for lexeme payloads that need Unicode
  content
- intern semantic identifier names early, while preserving original lexeme text
  for diagnostics and future reformatting

This gives the compiler three things it does not have today:

1. source-faithful diagnostics for any token or AST node
2. a clean path to Unicode string literals
3. a way to add Unicode identifiers later without redesigning the compiler's
   text ownership model

## Current problem

The current code mixes two different concerns into `Lexeme.text`:

- source-facing text, used for diagnostics and retained in the AST
- semantic text, used for identifier lookup and literal parsing

Today that field is a `std::string`, and some span math depends on
`lexeme.text.size()`. That is workable for ASCII-only lexemes, but it is the
wrong foundation for general Unicode support.

The key failure modes are:

- byte length is not the same thing as source column width or codepoint count
- semantic operations like identifier lookup do not need the same
  representation as source-preserving lexeme text
- deferring interning makes later IR and symbol-table changes more invasive
- future AST-to-formatted-code output will need stable access to original token
  text and exact source ranges

## Requirements

The string design must satisfy all of these:

- Every lexeme must have a source-faithful span.
- Every AST node must continue to derive a faithful span from its constituent
  lexemes.
- The AST must retain enough information to emit formatted code in the future.
- The lexer must be able to store Unicode lexeme payloads now, especially for
  upcoming string literals.
- The immediate implementation does not need Unicode whitespace or Unicode
  identifiers.
- The design must leave room for Unicode identifiers later without replacing
  the core text model.
- Interning must be part of the architecture now.

## Recommendation

### 1. Make source text a first-class object

Introduce a source-file abstraction that owns the original bytes for a parsed
file. Lexemes and diagnostics should refer back to this source rather than
trying to reconstruct exact source positions from copied token strings.

The source object should support:

- immutable ownership of file contents
- translation between offsets and human-readable line/column positions
- slicing by span for diagnostics and later formatting work

This is the foundation that lets the AST preserve formatting-relevant source
information without forcing every token to duplicate enough metadata to recover
it later.

### 2. Define spans in terms of source positions, not token text length

`Source_span` should remain the primary range type, but lexeme spans should no
longer depend on `text.size()`.

The recommended direction is:

- lexing records the start and end source positions as each token is consumed
- `span_of(Lexeme)` returns the stored span directly
- AST `span_of(...)` functions continue to compose spans from child lexemes and
  nodes

This preserves the current AST model while removing the byte-counting bug from
the lexing layer.

### 3. Introduce a project-owned Unicode-capable text type

Replace naked `std::string` at syntax boundaries with a project-owned text
type. The recommended representation is UTF-8 storage with explicit source and
Unicode-aware helpers, not a whole-compiler switch to a codepoint container.

The text type should:

- own UTF-8 bytes
- support construction from lexer codepoints
- expose byte-oriented access for source slicing and symbol lookup
- expose codepoint iteration when needed
- make it explicit whether an operation is byte-based or codepoint-based

Why UTF-8 as the stored form:

- it matches the existing source encoding
- it minimizes churn in parser, AST, IR, diagnostics, and symbol lookup
- it keeps string literals and emitted source text naturally serializable
- it still allows Unicode-aware traversal where needed

This should be a compiler type, not a thin alias. The point is to force the
compiler to separate text ownership and text semantics intentionally.

### 4. Split source lexeme text from semantic identifier names

Not every consumer needs the same view of text.

The proposal should treat these as different things:

- source lexeme text: exact spelling as it appeared in source
- semantic identifier name: canonical name used for lookup and binding

For now those will often contain the same UTF-8 bytes, but the ownership model
should not assume they are the same forever.

That split is useful immediately:

- diagnostics and future formatting care about source spelling
- symbol lookup and IR care about stable semantic identity

### 5. Add identifier interning now

Identifier-like semantic names should be interned as part of the design, not as
future cleanup.

Recommended boundary:

- intern identifiers and keyword-like semantic names that participate in lookup
  or symbol identity
- do not intern punctuation tokens
- do not require numeric or string literal spellings to be interned

The intern table should:

- own canonical UTF-8 text for semantic names
- return a stable lightweight handle
- support cheap equality and hashing by handle

The AST should still preserve the original lexeme for diagnostics and source
reconstruction. The compiler can derive or cache an interned handle when a
lexeme is used as an identifier.

### 6. Keep lexemes in the AST

The current AST model, where nodes retain their constituent lexemes, is the
right direction and should remain stable.

For future formatted-code emission, retaining lexemes gives us:

- exact token spellings
- exact token order
- precise subnode spans

The proposal should not assume that spans alone are sufficient to regenerate
formatted code. Formatting will likely want both:

- AST structure
- access to original token/source text

Source ownership plus lexeme retention covers both needs.

## Proposed conceptual model

The intended future model is:

- `Source_file`: owns the original UTF-8 bytes and line index
- `Source_span`: closed range into a specific source file
- `Text`: owned UTF-8 text for lexeme payloads that need to survive beyond the
  source reader
- `Interned_string`: canonical semantic-name handle
- `Lexeme`: token kind, exact source span, and optional payload text where the
  token carries user text

The important separation is:

- spans tell us where something came from
- `Text` tells us what user-authored content the token carries
- interned handles tell us how semantic names are compared

## Why this is the right recommendation

This design is the best near-term fit for the current compiler because it:

- fixes the real source-fidelity problem at its root
- supports Unicode string literals without requiring a full codepoint-native
  rewrite
- preserves the current AST shape and span composition strategy
- lets symbol-table and IR code move to interned names in an incremental way
- avoids coupling source formatting fidelity to the semantic symbol model

## Rejected alternatives

### Keep `std::string` everywhere

Rejected because it keeps source fidelity, semantic identity, and Unicode
handling conflated. It also makes interning a later cross-cutting retrofit.

### Make all compiler text codepoint-native

Rejected as the primary design because it would force broad churn across
parsing, AST, IR, diagnostics, containers, and output code without solving the
main architectural problem better than source-owned UTF-8 plus explicit helpers.

Codepoint iteration is still important, but it should be a capability of the
text layer, not the storage format for every compiler string.

### Defer interning

Rejected because identifier text already crosses lexer, parser, AST, and IR
boundaries. Waiting longer increases the amount of code that will later need to
change shape around symbol identity.

## Migration plan

The expected implementation sequence after this proposal is accepted:

1. Introduce a source-file owner and make lexeme spans explicit.
2. Stop deriving lexeme span end positions from token text length.
3. Introduce the project-owned UTF-8 text type for lexeme payloads.
4. Add an interning facility and move identifier lookup paths to interned
   semantic names.
5. Add Unicode-capable string literal lexing and parsing on top of the new text
   model.
6. Revisit Unicode identifiers later as a policy choice, not a storage-model
   rewrite.

This order keeps source fidelity and Unicode lexeme storage ahead of policy
expansion.

## Risks and follow-on questions

These do not block the recommendation, but they should be tracked explicitly:

- Whether displayed diagnostic columns should remain codepoint-counted,
  byte-counted, or later evolve toward grapheme-aware presentation.
- How much trivia retention a future formatter will need beyond token spellings
  and spans.
- Whether interned handles should be attached directly to identifier lexemes or
  derived lazily by later stages.
- Whether string literal escape processing should store both raw spelling and
  decoded value.

## Decision

The compiler should move to a split model based on:

- source-owned UTF-8 bytes for fidelity
- explicit stored spans on lexemes
- a project-owned UTF-8 text type for Unicode lexeme payloads
- interned semantic identifier names

That is the recommended foundation for upcoming Unicode string literal work and
for future source-preserving tooling built on the AST.
