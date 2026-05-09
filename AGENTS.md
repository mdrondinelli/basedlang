# Agent Instructions

## Start here

At the start of a session, read [`docs/wiki/README.md`](docs/wiki/README.md) before making changes.

When the user asks you to make a plan for a task (e.g. from [`TODO.md`](TODO.md)), include in the plan:
* making a branch for the PR 
* formatting changed files
* building and running ALL tests
* committing changes, pushing the branch, and creating the PR

Separate generated or programmatic mechanical changes (formatting-only churn,
codegen, scripted rewrites such as Perl substitutions, etc.) into their own
commits instead of mixing them with behavioral or design changes.

## Commands

`clang-format-21 -i <path/to/source>`
`cmake --build build`
`./build/<executable-target>`

## PR comments

When posting GitHub PR comments with `gh`, write the comment body to a temporary
file first and pass it with `--body-file` instead of embedding the text directly
in the shell command.

Use your tools to author temporary PR comment files rather than shell commands.

When checking PR comments, prefer `gh api` over `gh pr view --comments`.
Current repo: `mdrondinelli/basedlang`.
Use these endpoints as needed:
* `repos/<owner>/<repo>/issues/<pr>/comments` for top-level PR conversation comments
* `repos/<owner>/<repo>/pulls/<pr>/reviews` for review summaries
* `repos/<owner>/<repo>/pulls/<pr>/comments` for inline review comments

This is more reliable here because `gh pr view --comments` can fail on deprecated
GraphQL fields.
