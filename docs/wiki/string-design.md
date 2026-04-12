# Compiler String Design Proposal

This document proposes how the compiler should represent token text, semantic
names, and source locations as the language grows beyond its current
ASCII-heavy surface.

It makes one concrete recommendation.

## Summary

Adopt a split model built around the lexer's codepoint boundary.

Here, "split model" means the compiler treats these as separate concerns even
when some of them may share storage in specific cases:

- source location: where syntax came from
- source spelling: how user-authored token text was written
- interned spelling: a canonicalized stored spelling that later phases can
  compare cheaply
- semantic meaning: what a use of that spelling resolves to in context

The recommended direction is:

- treat Unicode codepoints from `Char_stream` as the compiler's stable text
  input
- store explicit spans on lexemes instead of deriving span width from stored
  text length
- preserve original spelling for user-authored tokens so the AST can drive
  future pretty-printing
- keep escape decoding out of the lexer
- store variable-spelling token text in a shared reclaimable string table
- keep diagnostic snippet rendering outside the compiler core

This gives the compiler four things it does not have today:

1. accurate spans for any token or AST node
2. a path to Unicode string literals without committing the compiler to UTF-8
   internally
3. enough retained syntax text to support formatter-style source emission
4. shared token-text storage without per-lexeme heap churn

## Requirements

The string design must satisfy all of these:

- Every lexeme must have a faithful source span.
- Every AST node must continue to derive a faithful span from its constituent
  lexemes.
- The AST must retain enough information to drive pretty-printing later.
- The compiler must not require keeping the whole source file in memory.
- The compiler must not depend on the original input encoding after the source
  has been decoded into codepoints for `Char_stream`.
- The lexer must be able to preserve Unicode spelling in lexeme payloads.
- The immediate implementation does not need Unicode whitespace or Unicode
  identifiers.
- The design must leave room for Unicode identifiers later without replacing
  the core text model.
- String literal escape decoding must happen after lexing.
- Interning must be part of the architecture now.

## Recommendation

### 1. Treat codepoints as the compiler's text boundary

The compiler should not rely on source bytes or on UTF-8 specifically after
text enters `Char_stream`. The stable compiler-facing boundary is the stream of
Unicode codepoints already emitted by the character layer.

That means:

- lexer text storage should be defined in terms of codepoints or an
  encoding-agnostic owned representation derived from codepoints
- later compiler layers should not assume a particular source-file encoding
- output encoding should be chosen when pretty-printing or displaying text, not
  baked into token storage

The existing `Utf8_char_stream` remains a useful adapter for one input format,
but it should not define the compiler's internal string model.

### 2. Store spans explicitly on lexemes

`Source_span` should remain the core range type, but lexeme spans should no
longer be derived from token text length.

The recommended direction is:

- lexing records both start and end positions as each token is consumed
- `Lexeme` stores its span directly
- `span_of(Lexeme)` returns the stored span directly
- AST `span_of(...)` functions continue to compose node spans from child
  lexemes and nodes

This fixes the current length-based bug without changing the AST's overall
source-span model.

### 3. Preserve source spelling for user-authored token text

Future source emission is formatter-style reconstruction, not byte-for-byte
source replay. Even so, the AST needs the original spelling of tokens whose
text is chosen by the user.

Preserve original spelling for:

- identifiers
- numeric literals
- string literals and other future literal forms

Canonical regeneration is acceptable for:

- punctuation tokens
- keywords
- other fixed-spelling syntax

The proposal should recommend a project-owned token-text type for preserved
spellings. It should not just be a `std::string` typedef. The point is to make
source spelling an intentional compiler concept rather than an incidental byte
buffer.

This text type should:

- preserve the original sequence of codepoints seen by the lexer
- support equality, hashing, and output encoding at the edges
- avoid committing the compiler to one serialized encoding internally
- be represented as a move-only handle to table-owned spelling storage rather
  than an owned string buffer per lexeme

### 4. Keep token-text allocation cheap for common cases

The proposal should make allocation policy explicit.

The compiler should not perform a separate heap allocation for every lexeme that
has variable spelling.

Recommended policy:

- all variable-spelling token text goes through a shared string table
- identifiers are always interned, including short identifiers
- literals may use the same table-backed representation rather than lexeme-owned
  storage
- refcounts live on string-table rows
- fixed-spelling tokens such as punctuation and keywords carry no owned text
- rows are reclaimed when their non-atomic refcount drops to zero
- small spellings should avoid extra heap churn beyond the row allocation where
  practical

Short identifiers are not a good exception to interning. They are among the
most common and most repeated spellings in the program, so they benefit the
most from canonicalization and cheap handle-based comparison.

The main allocation goal is not "avoid interning short identifiers." It is
"avoid per-occurrence allocation and avoid heap churn for tiny payloads."

The string-table handle should have explicit ownership semantics:

- the handle is move-only
- moving transfers ownership without touching the refcount
- `.clone()` increments the row refcount and returns a new handle
- `.clone()` should be non-`const` so the API makes the non-threadsafe mutation
  obvious

This proposal assumes a single-threaded ownership model for these handles.

### 5. Keep the lexer responsible only for lexing

The lexer should preserve literal spelling, not interpret it.

For string literals in particular:

- the lexer stores the raw literal spelling
- the parser and AST retain that raw spelling
- later semantic compilation performs escape decoding and validation

This separation matters because formatting and diagnostics care about original
spelling, while semantic analysis cares about decoded values.

### 6. Separate source spelling, interning, and semantic meaning

The proposal should explicitly distinguish:

- source spelling: what the user wrote, preserved for formatting and
  source-facing behavior
- interned spelling: a canonicalized stored spelling reused across identical
  token texts stored in the string table
- semantic meaning: what a particular identifier occurrence resolves to during
  compilation

The lexer is not deciding what an identifier means. It is only producing lexemes
and assigning table-backed spelling handles as it lexes variable-spelling
tokens. Identifier lexemes should carry the same table-backed handle type used
for other variable-spelling tokens. Later semantic stages can then use those
interned identifier spellings for fast equality, hashing, and symbol-table
lookup.

For identifiers specifically, the implementation does not need to duplicate the
underlying text if one representation can serve both roles. In the common case,
a table-backed spelling handle can be the preserved spelling used for formatting
and also the canonical spelling key used by semantic lookup structures.

The reason to keep these concepts separate in the proposal is architectural, not
to force duplicate storage:

- literals already need preserved source spelling separate from semantic value
- future identifier policy may introduce normalization or alternate comparison
  rules
- formatting cares about "what was written"
- interning cares about "which spellings are textually the same"
- semantic analysis cares about "what this identifier occurrence denotes"

This gives the compiler room to:

- pretty-print using preserved user-authored spellings
- compare identifier spellings through interned handles
- evolve identifier policy later without redesigning formatting support

### 7. Add shared string-table storage now

Shared string-table storage should be part of the design now, not deferred.

Recommended boundary:

- attach table-backed spelling handles during lexing for every token that has
  variable user-authored text
- do not allocate text storage for punctuation, keywords, or other fixed
  spellings

The string table should:

- own canonicalized spellings in rows that carry a non-atomic refcount
- return stable lightweight handles
- reclaim rows when the last handle releases them
- support cheap equality and hashing by handle

The AST still keeps lexemes with preserved source spelling. For identifiers,
that preserved spelling should be represented by the same table-backed handle
carried on the lexeme and later used by lookup code. For literals and other
source-carrying tokens, the lexeme should carry the same kind of spelling
handle, but later semantic interpretation remains separate.

### 8. Keep diagnostics snippet rendering outside the compiler core

Diagnostics need accurate line/column positions and enough metadata to point
back into the original source, but the compiler does not need to keep full
source text in memory to do that.

The proposal should recommend:

- compiler diagnostics carry precise spans
- diagnostics also carry enough source identity or offset metadata for a caller
  to reopen or seek the original source later
- the outer presentation layer is responsible for reading lines, showing
  context, and adding ANSI highlighting

This keeps compiler data structures small while still supporting rich
diagnostics.

## Proposed conceptual model

The intended future model is:

- `Source_span`: a closed source range with explicit start and end positions
- `Interned_string`: move-only handle to string-table-owned preserved spelling for
  user-authored token text
- `Lexeme`: token kind, exact span, and optional preserved spelling for tokens
  that carry user-authored text

The important separation is conceptual:

- spans tell us where syntax came from
- preserved token text tells us how user-authored syntax was spelled
- interned spellings tell us which identifier texts are the same
- semantic analysis determines what those spellings mean in context

## Pretty-printing model

The formatter should reconstruct source from AST structure plus retained token
spellings, not from original source slices.

The target is canonical pretty-printing with spaces and newlines only.

That implies:

- original identifier and literal spellings remain available to print
- fixed-spelling syntax is emitted canonically from token kind or AST shape
- whitespace is regenerated by formatting rules, not preserved as trivia

This is enough to support source emission from AST nodes without requiring
whole-file source retention.

## Diagnostics model

Diagnostics and formatting have different needs and should not share one text
strategy.

Formatting needs:

- AST structure
- preserved spelling for user-authored tokens

Diagnostics need:

- accurate line and column numbers
- enough source identity and position data to reopen the original source and
  highlight the right range

The compiler should provide the second set and leave snippet extraction and
colorization to a higher layer.

## Why this is the right recommendation

This design is the best near-term fit for the current compiler because it:

- aligns the text model with the actual `Char_stream` boundary
- removes incorrect span-width assumptions
- supports upcoming Unicode string literal work without forcing a UTF-8-only
  internal representation
- preserves the AST's source-facing role for formatting
- keeps source spelling, interning, and semantic meaning cleanly separated
- introduces interning before more compiler layers depend on raw strings

## Rejected alternatives

### Keep `std::string` everywhere

Rejected because it keeps source spelling, interning concerns, semantic use, and
source-extent computation tangled together.

### Keep the whole source file in memory

Rejected because diagnostics do not require it, and formatter-style source
emission can be driven from AST structure plus retained token spellings.

### Make UTF-8 the compiler's canonical internal text model

Rejected because the compiler is already conceptually downstream of
`Char_stream`, and future source adapters should not be forced through one
serialized encoding choice after that boundary.

### Decode string literal escapes in the lexer

Rejected because it destroys original spelling too early and mixes lexical
tokenization with later semantic interpretation.

### Defer interning

Rejected because identifier text already crosses lexer, parser, AST, and IR
boundaries. Waiting longer only makes semantic-name migration broader.

## Migration plan

The expected implementation sequence after this proposal is accepted:

1. Add explicit spans to `Lexeme` and stop deriving span end positions from
   stored text length.
2. Introduce the shared string table, row refcounts, and move-only `Interned_string`
   handle with explicit `.clone()`.
3. Move variable-spelling lexeme payloads onto `Interned_string`, while keeping
   fixed-spelling tokens allocation-free.
4. Intern identifier spellings through the same string-table path and update
   lookup code to use handle-based equality and hashing.
5. Keep string literal tokens raw and move escape decoding to semantic
   compilation.
6. Build pretty-printing on AST structure plus preserved spellings and
   canonical whitespace rules.
7. Add richer source-backed diagnostic rendering outside the compiler core.
8. Revisit Unicode identifiers later as a policy decision rather than a storage
   redesign.

## Recommended implementation phases

To reduce risk, the implementation should be split into these PR-sized phases:

### Phase 1: Span correctness

- add explicit `Source_span` storage to `Lexeme`
- update `span_of(Lexeme)` and dependent code to use stored spans directly
- keep existing text storage temporarily so the source-location change lands on
  its own

### Phase 2: String table foundation

- add the shared string table
- add row-level non-atomic refcounts
- add the move-only `Interned_string` handle and non-`const` `.clone()`
- add tests for handle lifetime, move behavior, cloning, and reclamation

### Phase 3: Lexeme payload migration

- move identifier and literal spellings from raw owned strings to `Interned_string`
- keep punctuation and keywords allocation-free
- update parser and AST storage to carry the new handle type

### Phase 4: Identifier lookup migration

- switch symbol-table and semantic lookup paths to use the handle-based
  identifier spelling directly
- remove redundant identifier string comparisons where handle comparison is
  enough

### Phase 5: Raw string literal preservation

- ensure string literal tokens preserve raw spelling only
- move escape decoding and validation fully out of the lexer
- add tests that distinguish raw spelling retention from decoded semantic value

### Phase 6: Formatter-facing support

- build pretty-printing on AST structure plus preserved token spellings
- enforce canonical spaces/newlines policy
- verify identifiers and literals print from preserved spelling while fixed
  syntax prints canonically

### Phase 7: Diagnostic presentation integration

- keep compiler diagnostics span-based
- add or wire up an outer source-reading layer that reopens/seeks source for
  snippet rendering
- verify line/column reporting and highlight ranges remain correct

## Risks and follow-on questions

These do not block the recommendation, but they should be tracked explicitly:

- whether diagnostic columns should remain codepoint-counted or later move
  toward grapheme-aware presentation
- whether preserved token spelling should store codepoints directly or use
  another encoding-agnostic representation derived from them
- whether semantic decoding of string literals should retain both raw spelling
  and decoded value in later stages

## Decision

The compiler should move to a split model based on:

- codepoint-oriented compiler text after `Char_stream`
- explicit stored spans on lexemes
- preserved original spelling for user-authored token text
- deferred literal escape decoding
- interned identifier spellings
- external source access for rich diagnostic rendering

That is the recommended foundation for upcoming Unicode string literal work and
future formatter-style source emission from the AST.
