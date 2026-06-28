# SDD Harness Workflow

Date: 2026-06-15

## Intake

1. Read the user request.
2. Read `harness/README.md` and the relevant section docs.
3. Read any tool-specific local instructions if present.
4. Check `git status --short` and preserve existing user changes.

## Discovery

1. Identify the affected plugin or shared module.
2. Read local CMake, source, tests, and common interface files.
3. Prefer `rg` or `rg --files` for searches.
4. Confirm whether the behavior is deterministic or live-service dependent.

## Specification

1. For non-trivial behavior, write a short SDD note before editing code.
2. Include context, goal, non-goals, affected files, behavior, acceptance
   criteria, and harness plan.
3. Put task-specific notes in the tool's local workspace when required.
4. Put durable cross-agent behavior summaries under `harness/specs/`.

## Implementation

1. Make the smallest change that satisfies the spec.
2. Keep source behavior in the proper layer.
3. Add or update focused tests when behavior changes.
4. Avoid unrelated formatting or refactors.

## Verification

1. Run the narrowest useful harness first.
2. Use `--build-dir out/harness` for AI-agent harness verification so existing
   developer `build/` directories are not reused or modified.
3. Broaden to `Validate -EnableTest` when shared behavior, CMake, dependencies,
   or template behavior changed.
4. If a command cannot run, record the exact command and blocker.
5. Re-check `git status --short` after harness commands.

## Handoff

Report:

- changed files;
- verification command and result;
- skipped verification and exact reason;
- remaining risk, especially live-network or credential-dependent behavior.
