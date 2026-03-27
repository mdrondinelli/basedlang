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

### Type expressions

Type expressions use prefix modifiers applied to a base type:

| Modifier      | Syntax        | Example              | Meaning                           |
|---------------|---------------|----------------------|-----------------------------------|
| pointer       | `*T`          | `*Int32`             | pointer to (const) `Int32`          |
| mut pointer   | `*mut T`      | `*mut Int32`         | pointer to mutable `Int32`          |
| sized array   | `[expr]T`     | `[4]Int32`           | array of 4 `Int32`                 |
| unsized array | `[]T`         | `[]Int32`            | dynamically-sized array of `Int32s` |

These compose right to left. `*mut` marks the pointee as mutable.
Pointer/variable mutability comes from the binding, not the type.

Examples:

| Type expression       | Meaning                                                              |
|-----------------------|----------------------------------------------------------------------|
| `Int32`               | plain integer                                                        |
| `*Int32`              | pointer to `Int32`                                                     |
| `*mut Int32`          | pointer to mutable `Int32`                                             |
| `[4]Int32`            | sized array                                                          |
| `*[]Int32`            | pointer to unsized array                                             |
| `*mut []Int32`        | pointer to mutable unsized array                                     |
| `*[8]*mut [4]Int32`   | pointer to array of 8 of (pointer to mutable array of 4 of `Int32`)    |

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
