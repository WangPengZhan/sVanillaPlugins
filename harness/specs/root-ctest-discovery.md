# Root CTest Discovery

Date: 2026-07-20

## Context

Plugins register GoogleTest executables with `add_test`, but testing is enabled
only inside plugin subdirectories. CMake therefore generates plugin-local
`CTestTestfile.cmake` files without generating the root test file that CTest
needs when invoked through the repository harness.

## Goal

Make `ctest --test-dir out/harness` discover plugin tests whenever the shared
`ENABLE_TEST` option is enabled.

## Behavior

- The repository root owns the `ENABLE_TEST` option.
- Enabling it calls `enable_testing()` before plugin subdirectories are added.
- `TemplatePlugin`, which introduces shared runtime targets, is configured
  before plugins whose tests copy those targets beside their executables.
- Disabling it preserves the existing non-test build behavior.
- Plugin-local test definitions and GoogleTest executables remain unchanged.

## Acceptance Criteria

- Test-enabled configuration generates `out/harness/CTestTestfile.cmake`.
- Root CTest lists the BiliBili and YouTube unit-test registrations.
- The registered BiliBili and YouTube unit tests run successfully through
  root CTest.

## Harness Plan

Run configure, build the two focused unit-test targets, then execute filtered
root CTest from `out/harness`.
