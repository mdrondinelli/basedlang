# CLAUDE.md

## Commits

Prefer atomic commits: each commit should represent one logical change. When a task involves multiple distinct steps (e.g., a refactor followed by a new feature), split them into separate commits.

## Building

If `build/` doesn't exist yet:
```bash
cmake -B build -DCMAKE_CXX_COMPILER=clang++-21 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Then (or if `build/` already exists):
```bash
cmake --build build
```

## Checking for errors and warnings

Use `clangd-21 --check=<file>` to check a file for errors and warnings:

```bash
clangd-21 --check=<file>
```

The output is verbose. `E[` lines about tweak failures (e.g. `tweak: ExtractFunction ==> FAIL`) are clangd refactoring noise, not compiler errors — ignore them. Real compiler errors appear as diagnostic messages in the output. The final summary line `All checks completed, N errors` reflects only real diagnostics.

## Formatting

Use `clang-format-21` to format C++ files:

```bash
clang-format-21 -i <file>
```

## C++ coding style

**Braced initialization:**
- Prefer braced initialization everywhere (e.g., `auto x = int{};`, `auto s = std::string{};`)
- Exception: `std::vector<T>(n)` must use parentheses to call the size constructor — `std::vector<T>{n}` would use the initializer-list constructor instead

**Parameters:**
- Prefer pointers over non-const references for mutable parameters — it makes mutation visible at the call site (e.g., `foo(&x)` not `foo(x)`)

**Includes:**
- Standard library headers before project headers
- Use relative includes for headers within the same library (e.g., `#include "token.h"` not `#include "basedlex/token.h"`)

**Const:**
- Prefer `const` for local variables where possible (e.g., `auto const result = ...`)

**Auto:**
- Prefer `auto` for variable declarations (e.g., `auto x = int{};`, `auto const y = foo()`)
- For pointer-to-const results (e.g., from `dynamic_cast<T const*>`), use `auto const` rather than `auto const*` — `auto` deduces `T const*`, and the outer `const` makes the pointer itself const too

**Immediately invoked lambda expressions (IILEs):**
- Only use IILEs when they allow the result variable to be `const` — i.e., when computing a value requires internal mutation. Do not use an IILE if the variable cannot be `const` anyway.
  ```cpp
  auto const x = [&]() -> int
  {
    auto result = int{};
    for (auto const item : items)
    {
      result += item;
    }
    return result;
  }();
  ```

**Formatting:**
- No blank lines inside function bodies
- Prefer empty lines between function/method declarations in headers

**Testing:**
- Test files are named `*_test.cpp` (e.g., `lexeme_stream_test.cpp`)
- Shared test utilities (files that don't contain tests themselves) omit the suffix (e.g., `istream_binary_stream.cpp`)

**Naming:**
- `struct`, `class`, and `enum`: `Snake_case` with capital first letter (e.g., `Descriptor_set_layout`, `Work_recorder`)
- Functions, variables, namespaces, files: `snake_case`
- Private member variables: `_snake_case` with underscore prefix (e.g., `_header`, `_object`)
- Enum values: `snake_case`
- Template arguments: `PascaleCase`
