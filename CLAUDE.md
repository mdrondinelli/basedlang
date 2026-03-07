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

The output is verbose; the important lines start with `E[` (errors) or the final summary `All checks completed`.

## Formatting

Use `clang-format-21` to format C++ files:

```bash
clang-format-21 -i <file>
```

## C++ coding style

**Includes:**
- Standard library headers before project headers
- Use relative includes for headers within the same library (e.g., `#include "token.h"` not `#include "basedlex/token.h"`)

**Const:**
- Prefer `const` for local variables where possible (e.g., `auto const result = ...`)

**Auto:**
- Prefer `auto` for variable declarations (e.g., `auto x = int{};`, `auto const y = foo()`)

**Formatting:**
- No blank lines inside function bodies

**Naming:**
- `struct`, `class`, and `enum`: `Snake_case` with capital first letter (e.g., `Descriptor_set_layout`, `Work_recorder`)
- Functions, variables, namespaces, files: `snake_case`
- Private member variables: `_snake_case` with underscore prefix (e.g., `_header`, `_object`)
- Enum values: `snake_case`
- Template arguments: `PascaleCase`
