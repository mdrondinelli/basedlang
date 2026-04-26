# `ast`

`ast` is the AST module.

## Interface

The public surface is centered on:

- `Expression`
- `Statement`
- `Translation_unit`
- `Operator`
- `Source_span`

This module defines the data model that the parser produces and the compiler consumes.
`Source_span` itself is provided by `source`.

## Core abstractions

### `Expression`

The main AST sum type for expression forms. Each variant carries its constituent lexemes so that diagnostics can point back to the exact source location.

### `Statement`

The AST sum type for statements inside blocks.

### `Translation_unit`

The AST for the whole program: a sequence of top-level let bindings.

### `Operator`

Semantic operator identity. Decoupled from token identity so that the same operator enum can be used for precedence rules during parsing and for overload dispatch during compilation.

### `Source_span`

A closed source range over a pair of `Source_location` values. Span utilities are provided for all AST node types so that diagnostics can compute ranges without re-walking the tree.

## Data model

The AST is explicit:

- each syntax form has its own struct
- lexemes are preserved throughout the tree
- recursive links use `std::unique_ptr`
- sum types use `std::variant`

That keeps diagnostics source-facing and makes later compilation code easy to line up with syntax.

## What to keep stable

- all AST nodes must preserve their constituent lexemes
- `Operator` must remain decoupled from token identity
- `Source_span` must cover the full source extent of each node form
