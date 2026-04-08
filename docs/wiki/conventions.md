# Conventions

This page collects the conventions that apply across all C++ code in the project.
Formatting is handled separately — see [Formatting](./formatting.md).

## Naming

| Category | Convention | Examples |
|---|---|---|
| `struct`, `class`, `enum` | `Snake_case` (capital first letter) | `Lexeme_stream`, `Type_pool` |
| Functions, variables, namespaces, files | `snake_case` | `compile_expression`, `type_of_register` |
| Private member variables | `_snake_case` (underscore prefix) | `_header`, `_register_types` |
| Template parameters | `PascalCase` | `Key`, `Value` |
| Error types | suffix with `_error` | `Lex_error`, `Decode_error` |

## Initialization

Prefer braced initialization everywhere:

```cpp
auto x = int{};
auto s = std::string{};
auto v = std::vector<int>{1, 2, 3};
```

Exception: use parentheses to call the size constructor on `std::vector`, because
`std::vector<T>{n}` invokes the initializer-list constructor instead:

```cpp
auto v = std::vector<int>(n);   // correct: n elements
auto v = std::vector<int>{n};   // wrong: one element with value n
```

Use `{}` for default member initializers, not `= nullptr` or `= 0`:

```cpp
Type *_ptr{};
int _count{};
```

## `auto` and `const`

Prefer `auto` for variable declarations. Prefer `const` for locals where possible:

```cpp
auto const result = compile_expression(...);
auto const n = items.size();
```

For const-pointer results (e.g. from `dynamic_cast<T const*>`), use `auto const`,
not `auto const*` — `auto` already deduces the pointer type, and the outer `const`
makes the pointer itself const too:

```cpp
auto const node = dynamic_cast<Int_literal_node const*>(expr);
```

## Parameters

Prefer pointers over non-const references for mutable parameters. This makes
mutation visible at the call site:

```cpp
void update(Foo *foo);   // preferred: foo->... is visible at call site
void update(Foo &foo);   // avoid: mutation is invisible at call site
```

At interfaces, prefer `std::span` over `const std::vector&` for read-only array
parameters, and prefer `std::int32_t` over `int` for sizes and counts:

```cpp
auto compile(std::span<Token const> tokens, std::int32_t limit) -> Result;
```

## Immediately-invoked lambda expressions (IILEs)

Only use an IILE when it allows the result variable to be `const` — i.e., when
computing a value requires internal mutation but the final result is fixed:

```cpp
auto const total = [&]() -> int
{
  auto result = int{};
  for (auto const item : items)
  {
    result += item;
  }
  return result;
}();
```

Do not use an IILE if the variable cannot be `const` anyway — it adds noise without benefit.

## Includes

Standard library headers before project headers. Use relative includes for headers
within the same library:

```cpp
#include <cstdint>
#include <vector>

#include "token.h"      // relative, not <kalelex/token.h>
```

## Formatting conventions

- No blank lines inside function bodies.
- Blank lines between function and method declarations in headers.
