# Verification Matrix

Date: 2026-06-15

Run commands from the repository root.

## Commands

```bash
python scripts/harness.py --action configure --enable-test
python scripts/harness.py --action build --enable-test
python scripts/harness.py --action test --enable-test
python scripts/harness.py --action validate --enable-test
python scripts/harness.py --action format
```

## Selection

| Change Type | First Harness | Broaden When |
| --- | --- | --- |
| Markdown or shared AI docs only | `Format` | Usually not needed; confirm no source formatter side effects. |
| Formatting-only source change | `Format` | Build if formatting exposed compile-sensitive generated code. |
| Source-only plugin change | `Build -EnableTest` | Run `Test -EnableTest` if behavior or tests changed. |
| URL parser behavior | `Build -EnableTest` | Run `Test -EnableTest`; prefer focused parser tests. |
| Test change | `Configure -EnableTest`, `Build -EnableTest`, `Test -EnableTest` | Run `Validate -EnableTest` for shared test infra. |
| CMake change | `Validate -EnableTest` | Document generator or platform-specific limits. |
| Dependency change | `Validate -EnableTest` | Expect vcpkg/network limitations to be documented. |
| `TemplatePlugin` or common contract change | `Validate -EnableTest` | Review every affected plugin if ABI changed. |
| Live-service behavior | `Build -EnableTest` plus targeted test/manual check | Document network, credential, or platform instability. |

## Failure Handling

1. Decide whether the failure is caused by the current change.
2. Fix failures caused by the current change.
3. Leave unrelated pre-existing failures unchanged.
4. Report the failing command and most useful failure line.
