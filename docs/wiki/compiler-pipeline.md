# Compiler pipeline

This page explains how information moves through the system and which abstractions matter in each stage.

## Input and command-line layer

The `based` executable is thin on purpose. Its job is to:

- load source text
- build the lexer, parser, and compiler pipeline
- compile the whole translation unit
- find the requested function by name
- interpret it with command-line arguments

If this layer gets complicated, that is usually a smell. Most behavior belongs in one of the libraries.

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

### Reviewer questions

- Does the tokenization decision belong in lexing rather than parsing?
- Are source locations still precise?
- If a token was added or changed, were parser expectations updated consistently?
- If lookahead behavior changed, is buffering still correct and stable?

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
- punctuation is usually retained as `Lexeme`
- recursive relationships are represented with `std::unique_ptr`
- sum types are represented with `std::variant`

This design makes diagnostics and source-span recovery easier, and it keeps later compiler stages close to source syntax.

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

- Is name resolution correct across scopes and barriers?
- Are type identities canonicalized through `Type_pool` rather than recreated ad hoc?
- Is a compile-time value being treated as an object, or vice versa?
- Does the emitted HLIR preserve the semantics of the AST form being compiled?
- Are diagnostics emitted at the right source span?
- If operator behavior changed, did both typing and evaluation change consistently?

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

