# Git Standard

## Branches And Commits

- Keep each commit focused on one clear purpose.
- Do not mix formatting, refactoring, and behavior changes in the same commit
  unless the change is small and tightly related.
- Do not commit generated output from `build/`, `out/`, temporary files, or
  platform-generated files.
- Preserve existing user changes. Do not revert or overwrite them unless the
  user explicitly asks for that.
- Treat `TemplatePlugin` as a submodule when reviewing status or preparing
  commits.

## Commit Messages

Recommended format:

```text
type(scope): summary
```

Common types:

- `feat`: new behavior or capability.
- `fix`: bug fix.
- `refactor`: behavior-preserving code restructuring.
- `test`: test additions or updates.
- `docs`: documentation changes.
- `build`: CMake, dependency, or build workflow changes.
- `chore`: maintenance that does not fit the types above.

Examples:

```text
fix(dedao): avoid duplicate whole-archive links
docs(harness): require isolated out harness builds
```

## Pre-Commit Checks

- Run `git status --short`.
- Review the diff and confirm it only contains changes for the current task.
- Run the repository formatter before committing:

```bash
python scripts/clang_format_all.py
```

- Run the narrowest relevant harness command and record the result in the
  commit notes, PR, or final handoff.
- Use `--build-dir out/harness` for configure, build, test, and validate
  harness commands unless the user explicitly requests another directory.
