# Antipatterns for reviewers

This page is intentionally present early so reviewer guidance has a stable home in the wiki.

Detailed antipatterns have not been filled in yet.

## Placeholder categories

- semantic fixes implemented in the wrong layer
- parser changes that silently alter precedence or associativity
- interpreter logic compensating for invalid HLIR
- ad hoc type construction outside `Type_pool`
- diagnostics that point at derived nodes instead of user syntax
- tests that only cover the happy path of a new feature
- PRs that change maintainer mental models without updating the wiki

## How to use this page for now

When you add a new antipattern:

1. describe the smell
2. explain why it is dangerous in this codebase specifically
3. describe what a better implementation usually looks like
4. mention what tests or docs should accompany the fix
