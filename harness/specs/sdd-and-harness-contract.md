# Spec: SDD And Harness Contract

Date: 2026-06-15
Status: Current

## Context

The repository uses a spec-first engineering loop combined with repeatable
harness commands. This contract applies to AI agents and human maintainers when
work changes behavior, build logic, tests, or shared project rules.

## SDD Rule

Write or update a short spec before implementing non-trivial changes.

Use SDD for:

- user-visible behavior changes;
- URL parser behavior;
- login, download, or conversion behavior;
- build system or dependency changes;
- shared API, ABI, or cross-plugin changes;
- new plugin scaffolding;
- repository workflow changes.

Tiny mechanical edits can skip a separate spec, but the final handoff still
needs a verification result.

## Spec Contents

A useful spec states:

- context;
- goal;
- non-goals;
- affected files;
- behavior;
- acceptance criteria;
- harness plan;
- notes or risks.

Tool-specific task specs can live outside this tree. Shared durable specs should
be summarized under `harness/specs/`.

## Harness Rule

Use `scripts/harness.py` from the repository root for verification:

```bash
python scripts/harness.py --action configure --enable-test --build-dir out/harness
python scripts/harness.py --action build --enable-test --build-dir out/harness
python scripts/harness.py --action test --enable-test --build-dir out/harness
python scripts/harness.py --action validate --enable-test --build-dir out/harness
python scripts/harness.py --action format
```

AI-agent verification should use `out/harness` as the harness build directory.
Do not use a developer's existing `build/` directory unless the user explicitly
asks for that directory.

Choose the narrowest useful command first. Broaden verification when shared
behavior, build logic, dependencies, or template behavior changed.

## Final Evidence

Final handoff should include:

- changed files;
- command run;
- pass/fail result;
- key failure line if failed;
- skipped commands and exact reason;
- remaining risk.
