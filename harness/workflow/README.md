# Workflow

Use this section for the shared work process that AI agents should follow.

Default loop:

1. Read relevant shared context from `harness/`.
2. Read any tool-specific local instructions if present.
3. Inspect the affected source, tests, and build files.
4. Write or update requirements/spec notes for non-trivial behavior.
5. Implement the smallest scoped change.
6. Run the narrowest useful verification command.
7. Report changed files, verification result, and remaining risk.

For this repository, verification should use the cross-platform harness:

```bash
python scripts/harness.py --action format
python scripts/harness.py --action configure --enable-test
python scripts/harness.py --action build --enable-test
python scripts/harness.py --action test --enable-test
python scripts/harness.py --action validate --enable-test
```

Choose the smallest command that matches the change risk, then broaden when
shared behavior, build logic, or runtime contracts are touched.

## Current Files

- `sdd-harness-workflow.md`: shared SDD plus harness workflow for AI agents.
- `verification-matrix.md`: command selection by change type.
