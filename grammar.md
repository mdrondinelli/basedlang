# based grammar

## Program structure

A program is a sequence of function definitions. Each is a `let` binding of a
`fn` expression, terminated by a semicolon:

```
let main = fn() -> i32 {
  return 0;
};
```

## Statements

Statements appear inside block bodies (`{ ... }`):

- `let x = <expr>;` — variable binding
- `return <expr>;` — return from function
- `<expr>;` — expression statement

## Expressions

Precedence from tightest to loosest (all binary operators are left-associative):

| Precedence | Kind           | Examples                |
|------------|----------------|-------------------------|
| 0          | postfix        | `f()`, `arr[i]`         |
| 1          | unary          | `*p`, `+x`, `-x`       |
| 2          | multiplicative | `a * b`, `a / b`, `a % b` |
| 3          | additive       | `a + b`, `a - b`        |

Primary expressions:

- Integer literals: `42`
- Identifiers: `x`
- Parenthesized: `(expr)`
- Block expressions: `{ statements... tail_expr }`
- Function expressions: `fn(params) -> ReturnType { body }`
- Constructor expressions: `new Type { args }`

Call and index are postfix and can chain: `f()(0)[1]`

The return type (`-> Type`) is optional on `fn` expressions. Parameters support
trailing commas. Parameters are immutable by default; prefix with `mut` for a
mutable binding: `fn(mut x: i32) { }`.

## Type expressions

A type starts with an identifier and can be followed by any number of postfix
modifiers:

| Modifier   | Syntax       | Example             | Meaning                       |
|------------|--------------|---------------------|-------------------------------|
| sized array | `[expr]`    | `i32[4]`            | array of 4 i32s               |
| unsized array | `[]`     | `i32[]`             | dynamically-sized array of i32s |
| pointer    | `*`          | `i32*`              | pointer to (const) i32        |
| mut pointer | `mut*`      | `i32 mut*`          | pointer to mutable i32        |

These chain left to right. `mut` only appears before `*` — it marks the pointee
as mutable. Pointer/variable mutability comes from the binding, not the type.

Examples:

```
i32                 -- plain integer
i32*                -- pointer to i32
i32 mut*            -- pointer to mutable i32
i32[4]              -- sized array
i32[]*              -- pointer to unsized array
i32[] mut*          -- pointer to mutable unsized array
i32[4] mut*[8]*     -- pointer to array of 8 of (pointer to mutable array of 4 of i32)
```

## Full examples

```
-- function with parameters and return type
let add = fn(a: i32, b: i32) -> i32 {
  return a + b;
};

-- function taking a pointer to a mutable array
let zero_fill = fn(buf: i32[] mut*, len: i32) -> void {
  -- ...
};

-- constructing a value
let v = new Vec3{1, 2, 3};

-- constructing a sized array
let arr = new i32[3]{10, 20, 30};

-- nested calls and indexing
let x = get_buffer()[i + 1];

-- block expression (last expression without ; is the value)
let x = {
  let a = 1;
  let b = 2;
  a + b
};

-- block with no tail expression (produces void)
{ do_something(); };

-- dereference
let val = *p;
let first = *buf[0];

-- unary and binary operators
let y = -x + 2 * (a - b);
```

## Keywords

`fn`, `let`, `mut`, `new`, `return`
