# `spelling`

`spelling` is the shared preserved-spelling storage module for the compiler
front end.

## Interface

The public surface is currently centered on:

- `Spelling`
- `Spelling_table`
- `Spelling_table::Builder`

Later phases of the string-storage refactor will thread these types through
lexing, parsing, the AST, and semantic compilation.

## Core abstractions

### `Spelling`

A compact handle to a preserved spelling stored in a `Spelling_table`.

It does not own the spelling bytes and is only meaningful relative to the table
that created it.

### `Spelling_table`

Owns shared storage for preserved variable spellings.

It deduplicates equal spellings, stores their UTF-8 bytes, and resolves
`Spelling` handles back to `std::string_view`.

### `Spelling_table::Builder`

A lightweight builder used to construct one spelling incrementally in table
storage and then seal it with `finalize()`.

Destroying an unfinished builder rolls back the in-progress build.

## Data model

The current implementation is intentionally small and phase-2-oriented:

- one contiguous byte buffer for committed spellings and the active build
- one entry array storing offset and length for each spelling
- one open-addressed hash table that deduplicates spellings by content

This module does not manage spelling lifetimes beyond the lifetime of the table
itself. Callers must keep the table alive for as long as stored handles are
still in use.

## What to keep stable

- `Spelling` remains a compact non-owning handle
- `Spelling_table` remains the owner and lookup authority
- builder/finalize remains the path for constructing new spellings
- later phases may change who stores `Spelling`, but not what the handle means
