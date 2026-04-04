# Compiler pipeline

This page explains how information moves through the system and which abstractions matter in each stage.

## Input and command-line layer

The `based` executable is thin on purpose, and at this point it is basically a throwaway tool. It is useful for wiring the current pipeline together and exercising the language, but it should not be treated as a stable architectural center.

Its job today is to:

- load source text
- build the lexer, parser, and compiler pipeline
- compile the whole translation unit
- find the requested function by name
- interpret it with command-line arguments

This is mainly a convenience path for development. We probably will not keep executing HLIR directly in this form as the long-term product direction.

If this layer gets complicated, that is usually a smell. Most behavior belongs in one of the libraries, not in the executable wrapper.

## Lexing

### Main abstractions

- `Char_stream`: abstract Unicode character source
- `Char_stream_reader`: buffered lookahead over characters
- `Lexeme_stream`: stateful lexer that produces `Lexeme`
- `Lexeme_stream_reader`: buffered lookahead over lexemes
- `Lexeme`: token kind, original text, and source location
- `Token`: token enum for the language surface

### Why this matters

The parser depends on lexeme lookahead, so the lexer layer is built around readers rather than one-shot tokenization.

### How lexing is coded

The lexer implementation is very direct. `Lexeme_stream` is the core piece: each time the parser asks for another token, it runs a lexing routine that looks at the next character, decides what kind of token starts there, consumes the needed input, and returns one `Lexeme`.

The shape is basically:

1. skip over trivia such as whitespace while updating source location
2. look at the next character
3. enter a large branch or switch-style dispatch based on that character
4. consume as many following characters as belong to that token
5. build a `Lexeme` from the token kind, captured text, and starting location

So in practice the lexer is not a table-driven system or a separate generated state machine. It is hand-written control flow with a central tokenization loop and a big token-classification dispatch.

Identifier-like text is read by consuming characters until the identifier stops, then the resulting text is checked to decide whether it is:

- a keyword
- a plain identifier
- a composite keyword-like token such as `&mut` or `^mut`

Punctuation-heavy tokens are recognized by peeking ahead and taking the longest valid match, which is how the lexer distinguishes things like:

- `=` from `==`
- `!`-started sequences such as `!=`
- `<` from `<=`
- `>` from `>=`
- `=`/`>` combinations such as `=>`

The reader types support that implementation style:

- `Char_stream_reader` gives the lexer cheap character lookahead while it is inside that loop and dispatch logic
- `Lexeme_stream_reader` gives the parser token lookahead on top of the lexer

That means the parser never works with raw characters. By the time parsing starts, token boundaries and token kinds have already been decided by the hand-written logic in `Lexeme_stream`.

## Parsing

### Main abstractions

- `Parser`: recursive-descent parser with precedence handling
- `Translation_unit`: top-level list of `let` statements
- `Statement`: variant over the supported statement forms
- `Expression`: variant over all expression forms
- `Operator`: semantic operator identity used for precedence and later compilation logic

### AST shape

The AST is intentionally explicit:

- every syntax form has its own struct
- lexemes are preserved throughout the tree
- recursive relationships are represented with `std::unique_ptr`
- sum types are represented with `std::variant`

This design makes diagnostics and source-span recovery easier, and it keeps later compiler stages close to source syntax.

### Parsing algorithm

Parsing is hand-written recursive descent on top of `Lexeme_stream_reader`.

At a high level, the parser works like this:

1. inspect the next lexeme
2. choose the parse function for the syntactic form that can start there
3. consume required lexemes with `expect(...)`
4. recurse into child nodes
5. build explicit AST structs that retain the consumed lexemes

Expression parsing uses precedence climbing rather than one function per precedence level. The parser first reads a primary expression, then repeatedly checks whether the next token is a postfix or binary operator with enough precedence to continue. That is what lets it parse combinations like calls, indexing, prefix operators, and binary operators without a huge tower of nearly identical functions.

So the parser is still very direct code, but the expression parser uses precedence-driven looping to handle operator structure cleanly.

### Important expression families

- literals and identifiers
- function expressions
- parenthesized expressions
- prefix, postfix, and binary expressions
- call and index expressions
- block expressions
- if expressions
- type-construction syntax expressed as ordinary expressions

### Reviewer questions

- Is the AST preserving enough syntax to support diagnostics?
- Was precedence or associativity changed in one place but not another?
- Did a new syntax form get threaded through all parse entrypoints?
- Is the parser still syntax-only, or is semantic logic leaking in?

## Semantic compilation and HLIR construction

This is where most project complexity lives.

### Main abstractions

- `Compilation_context`: orchestrates semantic analysis and HLIR emission
- `Symbol_table`: resolves names across nested scopes
- `Type_pool`: interns and canonicalizes types
- `Type`: canonical runtime representation of types
- `Constant_value`: compile-time value domain
- `Unary_operator_overload` and `Binary_operator_overload`: operator semantics
- `Translation_unit` in HLIR: compiled functions plus lookup tables
- `Function`, `Basic_block`, `Instruction`, `Terminator`: executable HLIR

### What compilation is responsible for

- seeding builtin values such as `Int32`, `Bool`, `Void`, and the type universe
- resolving identifiers
- distinguishing constant values from object bindings
- evaluating compile-time expressions where required
- checking types and mutability
- lowering structured AST into blocks, registers, instructions, and terminators
- producing diagnostics with source spans

### Registers and operands

HLIR instructions mostly work in terms of:

- `Register`: a typed temporary slot
- `Operand`: either a `Register` or a `Constant_value`

That distinction matters because the compiler can fold constant expressions while still producing executable IR for non-constant computation.

### Control flow

Control flow is represented by:

- `Basic_block`: ordered instructions plus a terminator
- `Jump_terminator`: unconditional transfer with block arguments
- `Branch_terminator`: conditional transfer with separate argument lists
- `Return_terminator`: function exit

The block-argument design means values flow between blocks explicitly instead of relying on mutable ambient state.

### Reviewer questions

- Does the emitted HLIR preserve the semantics of the AST form being compiled?
- Are diagnostics emitted at the right source span?

## Interpretation

### Main abstractions

- `interpret(...)`
- `Fuel_exhausted_error`

### Execution model

The interpreter:

- allocates register storage for a compiled function
- seeds entry-block parameters from arguments
- executes instructions in order
- follows terminators between blocks
- returns a `Constant_value`

Interpretation uses a fuel counter to prevent runaway compile-time execution.

### Reviewer questions

- Does the interpreter match the HLIR contract exactly?
- If a new instruction or terminator is added, is interpretation updated?
- Is fuel consumed consistently for executable work?
