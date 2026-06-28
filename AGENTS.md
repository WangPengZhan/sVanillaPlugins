# Project Guide

This repository contains C++20 shared-library plugins for the sVanilla plugin
system. Changes in this repository must follow the shared spec-first workflow
and finish with an explicit harness result.

## Operating Model

Use the combined SDD + harness engineering loop:

1. Read the relevant plugin code, CMake files, tests, and common interfaces.
2. Write or update a short spec under `harness/specs/` for non-trivial shared
   behavior, or a task note in the relevant local documentation area.
3. Implement the smallest change that satisfies the spec.
4. Run the narrowest useful harness first, then broaden if shared behavior was
   touched.
5. Report changed files, verification commands, and any remaining risk.

For tiny mechanical edits, a separate spec file is optional, but the final
answer still needs the harness result.

Before changing code, read the relevant shared context:

- `harness/README.md` for the shared AI harness index.
- `harness/knowledge/project-overview.md` for the current repository
  architecture.
- `harness/requirements/current-repository-requirements.md` for engineering
  constraints and invariants.
- `harness/standards/engineering-standards.md` for team conventions and
  implementation standards.
- `harness/standards/git.md` for commit splitting and message conventions.
- `harness/standards/self-test.md` for verification expectations and result
  records.
- `harness/workflow/sdd-harness-workflow.md` for the SDD + harness loop.

## Repository Shape

- Plugin directories are top-level folders with their own `CMakeLists.txt`.
- Most plugins use `src/<ServiceApi>/`, `src/Plugin/`, and `test/plugin/`
  plus `test/unit/`.
- `TemplatePlugin` is the reference structure for new plugins.
- Root `CMakeLists.txt` adds every top-level directory containing a
  `CMakeLists.txt`.
- Dependencies are resolved through `ThirdParty/vcpkg` and `vcpkg.json`.

## Coding Rules

- Keep changes scoped to the requested plugin or shared module.
- Prefer existing plugin patterns over new abstractions.
- Use C++20 consistently with the current codebase.
- Preserve the exported plugin lifecycle ABI from `TemplatePluginCall.cpp`.
- Keep plugin code split between platform API code and `plugin::IPlugin`
  adapter code.
- Do not modify generated build output under `build/` or `out/`.
- Do not vendor new dependencies without an explicit requirement.
- Keep `.clang-format` behavior intact and use the existing formatting script.

## Harness Commands

Use the cross-platform harness from the repository root:

```bash
python scripts/harness.py --action configure --enable-test --build-dir out/harness
python scripts/harness.py --action build --enable-test --build-dir out/harness
python scripts/harness.py --action test --enable-test --build-dir out/harness
python scripts/harness.py --action validate --enable-test --build-dir out/harness
python scripts/harness.py --action format
```

If a command cannot be run in the local environment, state the exact reason and
the command that should be run later.

## Definition Of Done

A task is done only when:

- The relevant SDD spec or task note is created or updated when behavior changed.
- The implementation matches the acceptance criteria.
- The appropriate harness command was run, or the blocking reason is documented.
- User changes already present in the worktree were preserved.
