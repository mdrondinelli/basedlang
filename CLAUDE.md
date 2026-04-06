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

## See Also

[`AGENTS.md`](./agents.md)
