# CLAUDE.md

## Building

```bash
cmake -B build -DCMAKE_CXX_COMPILER=clang++-21 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

## Formatting

Use `clang-format-21` to format C++ files:

```bash
clang-format-21 -i <file>
```
