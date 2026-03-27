# based grammar

## Program structure

A program is a sequence of function definitions. Each is a `let` binding of a
`fn` expression, terminated by a semicolon:

```
let main = fn(): Int32 => {
  return 0;
};
```

## Statements

Statements appear inside block bodies (`{ ... }`):

- `let x = <expr>;` — immutable variable binding
- `let mut x = <expr>;` — mutable variable binding
- `return <expr>;` — return from function
- `while <expr> { }` — while loop
- `<expr>;` — expression statement

## Expressions


### Operators

Precedence from tightest to loosest (all binary operators are left-associative):

| Precedence | Kind           | Examples                |
|------------|----------------|-------------------------|
| 0          | postfix        | `f()`, `arr[i]`                                   |
| 1          | unary          | `&x`, `&mut x`, `*p`, `* T`, `*mut T`, `+x`, `-x` |
| 2          | multiplicative | `a * b`, `a / b`, `a % b`                         |
| 3          | additive       | `a + b`, `a - b`                                  |
| 4          | relational     | `a < b`, `a <= b`, `a > b`, `a >= b`              |
| 5          | equality       | `a == b`, `a != b`                                |
| 6          | assignment (right-assoc) | `a = b`, `a = b = c`                    |
 
### Primary Expressions

- Integer literals: `42`
- Identifiers: `x`
- Parenthesized: `(expr)`
- Block expressions: `{ statements... tail_expr }`
- If expressions: `if condition { } [else { }]`, `if cond { } else if cond { } else { }`
- Function expressions: `fn(params): ReturnType => expression`

The return type (`: Type`) is optional on `fn` expressions. Without a return
type, the syntax is `fn(params) => expression`. The body after `=>` is any
expression. Parameters support trailing commas. Parameters are immutable by
default; prefix with `mut` for a mutable binding: `fn(mut x: Int32) => { }`.

### Types as values

In basedlang, types are ordinary compile-time values. There is no separate
"type expression" grammar — what looks like a type annotation is just a regular
expression that the compiler requires to evaluate to a value of type `Type` at
compile time.

The builtin type names `Int32`, `Bool`, and `Void` are not keywords. They are
identifiers that are pre-declared in the root scope, each bound to a value of
type `Type`. They participate in the expression grammar exactly like any other
identifier. The compiler distinguishes them from runtime values not by syntax,
but by checking their type: an expression is a valid type annotation if and only
if it has type `Type` and can be evaluated at compile time.

This means that the operators `*`, `*mut`, `[]`, and `[n]` are not special type
syntax — they are ordinary prefix operators that happen to have overloads
accepting `Type` operands and producing `Type` results. The parser does not know
whether `*Int32` is a pointer type or a dereference; it parses both as a unary
`*` applied to a subexpression. The distinction is made later by the compiler,
which resolves the operator based on the type of the operand.

For example, consider a function parameter annotated with `*mut []Int32`. The
parser sees this as a chain of prefix operators applied to an identifier:

1. Parse the identifier `Int32`
2. Parse `[]` as a prefix bracket operator applied to `Int32`
3. Parse `*mut` as a prefix operator applied to `[]Int32`

The compiler then evaluates this expression at compile time:

1. Look up `Int32` — it's a `Type` value representing the 32-bit signed integer
2. Apply `[]` — produces a `Type` value representing an unsized array of `Int32`
3. Apply `*mut` — produces a `Type` value representing a mutable pointer to
   `[]Int32`

Because types are values, the same expression machinery handles all type
construction. The `[n]` operator accepts any compile-time constant integer
expression as the size, so `[2 + 2]Int32` is a valid type meaning "array of 4
`Int32`". No special constant-expression sublanguage is needed.

The prefix operators that construct types are:

| Operator | Meaning                                     |
|----------|---------------------------------------------|
| `*T`     | pointer to `T` (pointee is immutable)       |
| `*mut T` | pointer to `T` (pointee is mutable)         |
| `[n]T`   | sized array of `n` elements of type `T`     |
| `[]T`    | unsized (dynamically-sized) array of type `T` |

These compose naturally since each produces a `Type` value that can be the
operand of another type operator:

| Expression            | Meaning                                                            |
|-----------------------|--------------------------------------------------------------------|
| `Int32`               | 32-bit signed integer                                              |
| `*Int32`              | pointer to `Int32`                                                 |
| `*mut Int32`          | pointer to mutable `Int32`                                         |
| `[4]Int32`            | array of 4 `Int32`                                                 |
| `*[]Int32`            | pointer to unsized array of `Int32`                                |
| `*mut []Int32`        | pointer to mutable unsized array of `Int32`                        |
| `*[8]*mut [4]Int32`   | pointer to array of 8 of (pointer to mutable array of 4 of `Int32`) |

`*mut` marks the pointee as mutable. Pointer/variable mutability comes from the
binding, not the type — a `let mut` binding of a `*Int32` is a mutable pointer
to an immutable `Int32`.

## Full examples

Function with parameters and return type:

```
let add = fn(a: Int32, b: Int32): Int32 => {
  return a + b;
};
```

Function taking a pointer to a mutable array:

```
let zero_fill = fn(buf: *mut []Int32, len: Int32): Void => { };
```

Nested calls and indexing:

```
let x = get_buffer()[i + 1];
```

Block expression (last expression without `;` is the value):

```
let x = {
  let a = 1;
  let b = 2;
  a + b
};
```

Block with no tail expression (produces void):

```
{ do_something(); };
```

If/else expression:

```
let max = if a > b { a } else { b };
```

If without else (evaluates to void):

```
if done { cleanup(); };
```

Else-if chain:

```
let sign = if x > 0 { 1 } else if x < 0 { -1 } else { 0 };
```

While loop:

```
while n > 0 {
  n = n - 1;
}
```

Dereference and unary/binary operators:

```
let val = *p;
let first = *buf[0];
let y = -x + 2 * (a - b);
```

## Keywords

`else`, `fn`, `if`, `let`, `mut`, `return`, `while`
