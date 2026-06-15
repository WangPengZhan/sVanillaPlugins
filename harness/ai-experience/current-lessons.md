# Current AI Lessons

Date: 2026-06-15

## Start From The Smallest Affected Plugin

Most work should begin with the one plugin named by the task. Read the matching
`*Url`, `*Client`, `Convert`, `*Downloader`, `*Plugin`, test, and CMake files
before editing behavior.

## Keep Shared Context Tool-Neutral

`harness/` is for durable project context used by multiple agents and developer
tools. Tool-specific operational details should stay outside this tree, while
stable repository facts should be summarized here.

## Prefer Deterministic Tests

Several existing tests touch live websites. New tests should prefer pure URL
parsing, conversion, and error-handling coverage so the harness can distinguish
code regressions from network or platform changes.

## Avoid Accidental Build Participation

The root CMake file auto-adds every top-level directory containing a
`CMakeLists.txt`. Documentation directories such as `harness/` must not contain
one unless they intentionally become part of the build.

## Watch Formatter Side Effects

`python scripts/harness.py --action format` runs the repository formatter over
plugin source files. After running it for documentation-only work, check the
working tree and avoid carrying unrelated source formatting changes.
