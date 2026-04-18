# `streams`

`streams` is the raw input-stream module.

## Interface

The public surface is centered on:

- `Binary_stream`
- `Istream_binary_stream`
- `Char_stream`
- `Utf8_char_stream`
- `Char_stream_reader`

`lexing` consumes this module through `Char_stream` and `Char_stream_reader`.

## Core abstractions

### `Binary_stream`

Abstract source of raw bytes.
Its core operation is caller-owned bulk reads into a provided buffer.

### `Istream_binary_stream`

Adapter from `std::istream` to `Binary_stream`.

### `Char_stream`

Abstract source of Unicode codepoints.

### `Utf8_char_stream`

UTF-8 decoder from bytes to Unicode codepoints.

### `Char_stream_reader`

Character-level buffered lookahead.

## What to keep stable

- UTF-8 decoding stays incremental rather than preloading the whole file
- invalid UTF-8 continues to report `Utf8_char_stream::Decode_error`
- character lookahead remains separate from tokenization
