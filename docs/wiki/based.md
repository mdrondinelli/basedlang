# `based`

`based` is the command-line executable.

## Interface

It takes:

- a source file
- a function name
- optional function arguments

It then prints the interpreted result.

## Core responsibility

This module should stay thin. Its job is to:

1. load source text
2. build the lexer, parser, and compiler pipeline
3. compile the translation unit
4. look up the requested function
5. interpret it with command-line arguments

## Data model

There is very little local data model here. It is mostly a wiring layer between the libraries.

The only important values are:

- input path
- target function name
- parsed command-line arguments
- compiled translation unit

## Algorithm

The executable performs straightforward orchestration:

1. open file
2. create byte, character, lexeme, and parser pipeline objects
3. parse the translation unit
4. compile it
5. find the target function
6. convert CLI arguments into constant values
7. interpret and print the result

## What to keep stable

- keep it as a wrapper, not a new semantic layer
- avoid pushing compiler logic into the executable
- do not treat direct HLIR execution here as a permanent architectural commitment

