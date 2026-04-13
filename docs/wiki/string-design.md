# Compiler String Design Proposal

This proposal defines the refactor plan for moving the compiler away from
per-lexeme owned strings and toward shared preserved-spelling storage.

The main near-term goal is to reduce AST memory usage while preserving the
user-authored spelling needed by later source-facing features. Full source
retention and diagnostic line metadata are not part of this proposal.

## Current status

Phase 1 is already complete: lexemes now store explicit source spans directly.

The remaining work starts with introducing a shared spelling table and then
migrating lexer, parser, AST, and semantic code onto it.

## Phases

### Phase 1: Span correctness `(completed)`

- `Lexeme` stores explicit `Source_span` data.
- `span_of(Lexeme)` and AST span composition use stored spans directly.
- This phase is done and is included here only as context for the remaining
  refactor.

Acceptance:

- token spans are no longer derived from stored text length
- AST node spans remain faithful when composed from child nodes and lexemes

### Phase 2: Spelling-table abstraction

- Add a shared `Spelling_table` abstraction for preserved variable-spelling
  token text.
- The table provides an API for building new spellings in shared storage so the
  lexer can construct token text in place rather than through a per-lexeme
  owned string buffer.
- The table also interns completed spellings, deduplicates repeated spellings,
  returns stable compact `Spelling_id` values, and resolves `Spelling_id` back
  to stored spelling bytes.
- Use UTF-8 byte storage in this phase for memory efficiency.
- The caller is responsible for keeping the spelling table alive for as long as
  downstream compiler data structures still reference it.
- This phase does not yet change `Lexeme`, parser storage, AST storage, or
  semantic lookup.

Acceptance:

- equal spellings intern to the same `Spelling_id`
- different spellings intern to different `Spelling_id` values
- new spellings can be constructed through the table's builder/finalize path
  without an intermediate per-lexeme owned string
- stored spellings can be looked up through `Spelling_id`

### Phase 3: Lexeme and AST payload migration

- Remove per-lexeme owned string storage for variable-spelling tokens.
- Migrate identifiers and literal tokens to carry compact spelling references
  from `Spelling_table`.
- Keep punctuation, keywords, and other fixed-spelling syntax allocation-free
  and without preserved spelling payload.
- Update lexer, parser, and AST storage to use the new lexeme representation.
- Preserve raw literal spelling in the AST; semantic interpretation remains
  separate.

Acceptance:

- lexer, parser, and AST no longer require per-lexeme owned text for
  identifiers and literals
- fixed-spelling tokens do not allocate preserved spelling storage
- token and AST spans remain unchanged

### Phase 4: Semantic lookup migration

- Update AST-to-IR and symbol lookup paths to resolve identifier spelling
  through `Spelling_table`.
- Remove redundant `std::string`-based identifier ownership where spelling-table
  lookup is sufficient.
- Keep preserved spelling separate from semantic meaning.
- Move literal-handling paths off lexeme-owned text as part of the same
  migration.

Acceptance:

- semantic identifier lookup works through spelling-table resolution
- literal-handling paths no longer depend on lexeme-owned strings
- preserved spelling remains available where source-facing behavior needs it

## Decision

The compiler should move to shared preserved-spelling storage with this phase
order:

1. explicit spans on lexemes
2. shared spelling-table abstraction
3. lexeme and AST payload migration
4. semantic lookup migration

That is the planned string-storage refactor. Formatter support, diagnostic
rendering, and line metadata are separate follow-on work.
