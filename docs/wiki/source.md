# `source`

`source` is the header-only source-coordinate module.

## Interface

The public surface is centered on:

- `Source_location`
- `Source_span`
- `hull`

## Core abstractions

### `Source_location`

A 1-based line and column position in source code.

### `Source_span`

A closed source range over a pair of `Source_location` values.

### `hull`

Builds a composite span from the first location of one span and the last
location of another span.

## Ownership

This module owns source coordinate data only. It does not own tokenization,
diagnostics, parsing, or AST traversal.
