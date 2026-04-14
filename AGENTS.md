# Agent Instructions

## Start here

At the start of a session, read [`docs/wiki/README.md`](docs/wiki/README.md) before making changes.

When the user asks you to make a plan for a task (e.g. from [`TODO.md`](TODO.md)), include in the plan:
* making a branch for the PR 
* formatting changed files
* building and running ALL tests
* committing changes, pushing the branch, and creating the PR

## Commands

`clang-format-21 -i <path/to/source>`
`cmake --build build`
`./build/<executable-target>`
