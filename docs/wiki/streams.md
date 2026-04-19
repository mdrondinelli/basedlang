# `streams`

`streams` is the raw input-stream module.

## Interface

The public surface is centered on:

- `Binary_input_stream`
- `Istream_binary_input_stream`
- `Binary_input_stream_reader`
- `Char_input_stream`
- `Utf8_char_input_stream`
- `Char_input_stream_reader`
- `Lookahead_char_input_stream_reader`

`lexing` consumes this module through `Char_input_stream` and
`Lookahead_char_input_stream_reader`.

## Core abstractions

### `Binary_input_stream`

Abstract source of raw bytes.
Its core operation is caller-owned bulk reads into a provided buffer.

### `Istream_binary_input_stream`

Adapter from `std::istream` to `Binary_input_stream`.

### `Binary_input_stream_reader`

Byte-level buffered reader over a `Binary_input_stream`. Holds a 4 KiB
heap-allocated scratch buffer and refills it through the bulk
`read_bytes` API so callers amortize virtual dispatch.

### `Char_input_stream`

Abstract source of Unicode codepoints.
Its core operation is caller-owned bulk reads into a provided buffer.

### `Utf8_char_input_stream`

UTF-8 decoder from bytes to Unicode codepoints.

### `Char_input_stream_reader`

Character-level buffered sequential reader.

### `Lookahead_char_input_stream_reader`

Finite character lookahead over a `Char_input_stream`.

## What to keep stable

- UTF-8 decoding stays incremental rather than preloading the whole file
- invalid UTF-8 continues to report `Utf8_char_input_stream::Decode_error`
- character lookahead remains separate from tokenization
