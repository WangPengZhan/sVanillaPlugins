# Self-Test Standard

## Principles

- Self-tests must cover the main risk of the change, not only prove that the
  project compiles.
- Run the narrowest useful harness first, then broaden when the change affects
  shared behavior, CMake, dependencies, or template behavior.
- Use the repository harness from the repository root for repeatable
  verification.
- AI-agent configure, build, test, and validate runs must use `out/harness`.
  Do not reuse, clean, or modify a developer's existing `build/` directory.
- If automated verification is not possible, document the exact manual
  acceptance path.

## Common Commands

```bash
python scripts/harness.py --action configure --enable-test --build-dir out/harness
python scripts/harness.py --action build --enable-test --build-dir out/harness
python scripts/harness.py --action test --enable-test --build-dir out/harness
python scripts/harness.py --action validate --enable-test --build-dir out/harness
python scripts/clang_format_all.py
```

Use `python3` instead of `python` only when the local environment does not
provide a `python` executable.

## Result Records

Final replies and acceptance reports must record:

- the command;
- the result;
- the failure cause, if any;
- skipped verification and the exact reason;
- remaining risk.

If CTest reports `No tests were found!!!`, record that as the current test
discovery state instead of treating it as executed unit coverage.

## Manual Acceptance

For plugin loading, login, download, or live-service flows that are hard to
automate, write repeatable steps, expected results, and the observations used
to decide pass or fail.
