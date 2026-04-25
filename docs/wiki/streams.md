# `streams`

`streams` is the raw stream module.

It owns:

- byte input
- byte output
- optional byte buffering
- UTF-8 decoding
- character lookahead

`lexing` consumes this module through `Char_input_stream` and
`Char_input_stream_peeker`.

## Interface

The public surface is centered on:

- `Binary_input_stream`
- `Buffered_binary_input_stream<T>`
- `Output_stream`
- `Istream_binary_input_stream`
- `Posix_binary_input_stream` (POSIX-only)
- `Char_input_stream`
- `Utf8_char_input_stream`
- `Char_input_stream_peeker`

## Core abstractions

### `Binary_input_stream`

Abstract source of raw bytes.
Its core operation is caller-owned bulk reads into a provided buffer.

Concrete adapters such as `Istream_binary_input_stream` and
`Posix_binary_input_stream` live at environment boundaries and do not add extra
buffering on their own.

### `Buffered_binary_input_stream<T>`

Optional buffering adapter for a concrete `Binary_input_stream`.
It still presents the `Binary_input_stream` interface, but serves repeated
small reads from a 4 KiB scratch buffer and only goes back to the wrapped
stream when needed.

Use this when the provider wants byte-oriented callers to be efficient without
changing the consumer interface.

### `Output_stream`

Abstract sink for raw bytes.
Its core operation is caller-owned bulk writes from a provided buffer.
It also exposes `flush()`, which defaults to a no-op for unbuffered sinks and
can be overridden by buffered implementations.

### `Istream_binary_input_stream`

Adapter from `std::istream` to `Binary_input_stream`.

### `Posix_binary_input_stream`

POSIX-only adapter from a file descriptor to `Binary_input_stream`.
It is non-owning and does not close the fd.

### `Char_input_stream`

Abstract source of Unicode codepoints.
Its core operation is caller-owned bulk reads into a provided buffer.

It also provides a convenience `read_character()` helper implemented in terms
of the bulk API.

### `Utf8_char_input_stream`

UTF-8 decoder from bytes to Unicode codepoints.
It consumes a `Binary_input_stream` and exposes the `Char_input_stream`
interface.

This is where incremental UTF-8 decoding happens. The decoder does not preload
the whole input and continues to throw `Utf8_char_input_stream::Decode_error`
for invalid byte sequences.

### `Char_input_stream_peeker`

Finite character lookahead over a `Char_input_stream`.
It exposes `peek()` and `read()` and keeps lookahead state in a ring buffer.

This is the last stream-layer abstraction before lexing. Token lookahead stays
in `lexing`; character lookahead stays here.

## Typical composition

Common text-input pipelines now look like:

`Istream_binary_input_stream` or `Posix_binary_input_stream`
-> optional `Buffered_binary_input_stream<T>`
-> `Utf8_char_input_stream`
-> `Char_input_stream_peeker`
-> `lexing`

The provider chooses whether byte buffering is needed. Consumers continue to
depend on abstract `Binary_input_stream` or `Char_input_stream` surfaces.

## What to keep stable

- UTF-8 decoding stays incremental rather than preloading the whole input
- invalid UTF-8 continues to report `Utf8_char_input_stream::Decode_error`
- character lookahead remains separate from tokenization
- byte buffering remains an optional adapter, not a requirement of all byte
  sources
