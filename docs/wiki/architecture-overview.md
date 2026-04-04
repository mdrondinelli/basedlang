# Architecture overview

basedlang is currently a small compiler-plus-interpreter pipeline with four main layers:

1. `basedlex`: converts characters into lexemes.
2. `basedparse`: converts lexemes into an AST.
3. `basedhlir`: performs semantic compilation into a high-level IR, including type evaluation and compile-time execution.
4. `based`: command-line entrypoint that wires the pipeline together and interprets a selected function.

This split is the most important architectural fact in the project. Most changes should stay inside one layer. When a PR touches multiple layers, it should be easy to explain why the abstraction boundary had to move.

## End-to-end flow

At a high level, execution works like this:

1. The CLI opens a source file and turns bytes into Unicode characters.
2. The lexer groups characters into `Lexeme` values with token kind and source location.
3. The parser builds a syntax tree made from `Expression`, `Statement`, and translation-unit nodes.
4. The compiler walks the AST, resolves names, evaluates compile-time values, constructs canonical types, and emits HLIR functions and blocks.
5. The interpreter executes HLIR by evaluating instructions and following block terminators.

The semantic center of gravity is `basedhlir`. That layer owns:

- type identity
- symbol binding
- operator resolution
- diagnostics
- compile-time evaluation
- runtime interpretation of compiled functions

## What is unusual about this language

Two language ideas heavily shape the implementation:

### Types are values

Type syntax is not a separate world. Expressions like `Int32`, `^mut Int32`, and `[4]Int32` evaluate to values whose type is `Type`.

That means the compiler has to support:

- ordinary name lookup for builtin types
- compile-time evaluation of type expressions
- a clear distinction between object values, function values, and type values

### Functions can matter at compile time and run time

Function literals compile into HLIR functions and can also appear as compile-time constants via `Function_value`. This is why semantic code often has to reason about both:

- the type of a function
- the HLIR function object
- whether an expression is constant-evaluable

## Stable review boundaries

When reviewing or implementing changes, use these boundaries:

### Lexing boundary

Lexing should answer: "what token stream does this source produce?"

It should not answer parsing or typing questions.

### Parsing boundary

Parsing should answer: "what syntactic structure does this token stream represent?"

It should preserve source information and operator structure, but not perform semantic resolution.

### Semantic compilation boundary

Compilation should answer:

- what each identifier refers to
- which expressions are valid
- what types they have
- what HLIR instructions and blocks represent their behavior

### Interpretation boundary

Interpretation should answer: "given valid HLIR, what value does this function produce?"

It should not be compensating for malformed HLIR.

## What good changes look like

A clean PR in this project usually has these properties:

- the changed layer is obvious
- the invariants of that layer are preserved or intentionally extended
- tests move with the behavior change
- docs update when the architecture or maintainer mental model changes

