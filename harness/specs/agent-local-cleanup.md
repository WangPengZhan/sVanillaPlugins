# Spec: Agent Local Cleanup

Date: 2026-06-15
Status: Current

## Context

The shared `harness/` tree now contains the durable project knowledge,
requirements, standards, SDD workflow, and verification matrix. Older
tool-specific local files duplicated that information and could drift from the
shared source of truth.

## Goal

Keep durable engineering guidance in `harness/` and keep tool-specific files as
thin adapters only when they are required by a tool.

## Non-Goals

- Do not remove shared context from `harness/`.
- Do not add build participation to documentation directories.
- Do not change plugin runtime behavior.

## Behavior

Tool-specific instruction files may point to `harness/`, but should not copy the
same architecture, workflow, or verification content unless the tool requires a
local bootstrap file.

## Acceptance Criteria

- Shared engineering content remains available under `harness/`.
- Tool-specific files do not reference removed local context paths.
- Documentation directories remain build-neutral.
