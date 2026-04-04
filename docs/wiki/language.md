# Language reference

This page is the current language-surface reference.

## Status

### Fully implemented today

The features marked as implemented below are supported by the current parser and by `basedhlir`.

### Parsed or designed, but not fully implemented in `basedhlir`

These surface forms exist in the parser or language design, but are not fully supported semantically yet:

- `let mut` bindings parse, but mutable bindings are not implemented
- `while` statements parse, but are rejected in `basedhlir`
- postfix dereference `p^` parses, but dereference is rejected in expression compilation
- index expressions `arr[i]` parse, but index expression compilation is not implemented
- assignment expressions parse as operators, but are not implemented semantically
- omitted function return types parse, but return type deduction is not implemented

When future language work is still in flux, keep it separate from the fully implemented reference and label it clearly.

## Program structure

A program is currently a sequence of top-level `let` bindings. In practice, programs are usually built from named function definitions:

```based
let main = fn(): Int32 => {
  return 0;
};
```

Top-level constant evaluation is also implemented, so a top-level binding does not have to be a function.

## Statements

Statements appear inside block bodies:

- `let x = <expr>;`
- `return <expr>;`
- `<expr>;`

Parsed but not fully implemented in `basedhlir`:

- `let mut x = <expr>;`
- `while <expr> { ... }`

## Expressions

### Primary forms

Implemented primary expression forms:

- integer literals: `42`
- identifiers: `x`
- builtin boolean values via ordinary identifiers: `true`, `false`
- `recurse`
- parenthesized expressions: `(expr)`
- block expressions: `{ statements... tail_expr }`
- `if` expressions: `if condition { ... } else { ... }`
- function expressions with explicit return type: `fn(params): ReturnType => expression`

Parsed but not fully implemented in `basedhlir`:

- function expressions without an explicit return type

For example, this parses but return type deduction is not implemented yet:

```based
let id = fn(x: Int32) => x;
```

Function parameters are immutable by default. Parameter bindings may be written with `mut`:

```based
let f = fn(mut x: Int32): Void => { };
```

Parameters support trailing commas.

### Operators

Precedence from tightest to loosest:

| Precedence | Kind | Examples |
|---|---|---|
| 0 | postfix | `f()`, `arr[i]`, `p^` |
| 1 | prefix | `&x`, `&mut x`, `^T`, `^mut T`, `+x`, `-x` |
| 2 | multiplicative | `a * b`, `a / b`, `a % b` |
| 3 | additive | `a + b`, `a - b` |
| 4 | relational | `a < b`, `a <= b`, `a > b`, `a >= b` |
| 5 | equality | `a == b`, `a != b` |
| 6 | assignment | `a = b`, `a = b = c` |

Binary operators are left-associative except assignment, which is right-associative.

Parsed but not fully implemented in `basedhlir`:

- assignment expressions such as `a = b`

## Types as values

Types are ordinary compile-time values in the language.

There is no separate type-expression grammar. A type annotation is just an expression that must evaluate at compile time to a value of type `Type`.

The builtin type names `Int32`, `Bool`, and `Void` are ordinary identifiers predeclared in the root scope. They are not keywords.

This means expressions like these are all part of the ordinary expression system:

- `Int32`
- `^Int32`
- `^mut Int32`
- `[4]Int32`
- `[]Int32`

The compiler gives these forms type-construction meaning by resolving the relevant operator overloads on `Type` values.

### Type construction operators

Fully implemented type-construction operators:

| Operator | Meaning |
|---|---|
| `^T` | pointer to `T` |
| `^mut T` | pointer to mutable `T` |
| `[n]T` | sized array of `n` elements of `T` |
| `[]T` | unsized array of `T` |

These compose naturally:

| Expression | Meaning |
|---|---|
| `Int32` | 32-bit signed integer |
| `^Int32` | pointer to `Int32` |
| `^mut Int32` | pointer to mutable `Int32` |
| `[4]Int32` | array of 4 `Int32` |
| `^[]Int32` | pointer to unsized array of `Int32` |
| `^mut []Int32` | pointer to mutable unsized array of `Int32` |
| `^[8]^mut [4]Int32` | pointer to array of 8 of pointer-to-mutable-array-of-4-`Int32` |

`^mut` affects pointee mutability, not variable mutability. Variable mutability still comes from `let mut`.

The `[n]` form accepts any compile-time constant integer expression as the size, so `[2 + 2]Int32` is valid.

## Implemented examples

Function with parameters and return type:

```based
let add = fn(a: Int32, b: Int32): Int32 => {
  return a + b;
};
```

Function taking a pointer to a mutable array type:

```based
let zero_fill = fn(buf: ^mut []Int32, len: Int32): Void => { };
```

Nested calls:

```based
let x = first(id(42), 0);
```

Block expression:

```based
let x = {
  let a = 1;
  let b = 2;
  a + b
};
```

Block with no tail expression:

```based
{ do_something(); };
```

If / else expression:

```based
let max = if a > b { a } else { b };
```

Else-if chain:

```based
let sign = if x > 0 { 1 } else if x < 0 { -1 } else { 0 };
```

While loop:

```based
while n > 0 {
  n = n - 1;
}
```

This syntax parses, but `while` is not fully implemented in `basedhlir` yet.

Recursive call with `recurse`:

```based
let factorial = fn(n: Int32): Int32 => {
  return if n == 0 { 1 } else { n * recurse(n - 1) };
};
```

Dereference and unary/binary operators:

```based
let val = p^;
let first = buf[0]^;
let y = -x + 2 * (a - b);
```

Only the arithmetic and comparison forms here are fully implemented today. Dereference and indexing syntax exist, but are not fully implemented in `basedhlir`.

## Keywords

Implemented keywords:

`else`, `fn`, `if`, `let`, `mut`, `recurse`, `return`, `while`
