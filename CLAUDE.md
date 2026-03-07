# CLAUDE.md

## Building

If `build/` doesn't exist yet:
```bash
cmake -B build -DCMAKE_CXX_COMPILER=clang++-21 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Then (or if `build/` already exists):
```bash
cmake --build build
```

## Formatting

Use `clang-format-21` to format C++ files:

```bash
clang-format-21 -i <file>
```

## C++ coding style

**Naming:**
- `struct`, `class`, and `enum`: `Snake_case` with capital first letter (e.g., `Descriptor_set_layout`, `Work_recorder`)
- Functions, variables, namespaces, files: `snake_case`
- Private member variables: `_snake_case` with underscore prefix (e.g., `_header`, `_object`)
- Enum values: `snake_case`
- Template arguments: `PascaleCase`
