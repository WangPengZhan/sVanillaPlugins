# Shared AI Harness

This directory is shared project memory for AI agents and developers working on
this repository. It is intentionally tool-neutral so multiple tools can consume
the same context without copying it into tool-specific files.

The directory is documentation-only. Do not add `CMakeLists.txt` here.

## Reading Order

1. `workflow/` for the expected work loop.
2. `requirements/` for task and product intent.
3. `standards/` for project rules and quality bars.
4. `knowledge/` for durable facts about the codebase and domain.
5. `specs/` for behavior specs and design notes.
6. `ai-experience/` for lessons learned from previous AI-assisted work.

## Sections

- `ai-experience/`: accumulated AI collaboration notes, mistakes, fixes, and
  repeatable tactics.
- `knowledge/`: durable project, domain, architecture, and integration facts.
- `requirements/`: user goals, product requirements, and acceptance criteria.
- `specs/`: implementation-neutral behavior specs and design decisions.
- `standards/`: coding, testing, review, documentation, and safety rules.
- `workflow/`: end-to-end process for planning, implementation, verification,
  and handoff.

## Update Rules

- Keep entries short, dated when useful, and easy for agents to quote or apply.
- Prefer one focused document per topic instead of large mixed notes.
- Record durable facts here; keep temporary task scratchpads out of this tree.
- Tool-specific instructions may reference this directory, but shared project
  guidance should live here first.

## Current Project Index

Start here for the generated current-state content:

- `knowledge/project-overview.md`: repository architecture, build shape,
  plugin inventory, dependencies, and canonical docs.
- `knowledge/plugin-inventory.md`: current plugin list and supported URL
  surface.
- `requirements/current-repository-requirements.md`: product, compatibility,
  testing, and documentation requirements.
- `specs/plugin-architecture.md`: shared plugin layering and ABI contract.
- `specs/sdd-and-harness-contract.md`: SDD expectations and harness evidence
  rules.
- `standards/engineering-standards.md`: coding, build, dependency, testing,
  and safety standards.
- `standards/git.md`: commit splitting, message format, and pre-commit checks.
- `standards/self-test.md`: self-test scope, harness commands, and result
  records.
- `workflow/sdd-harness-workflow.md`: AI work loop from intake to final report.
- `workflow/verification-matrix.md`: harness command selection.
- `ai-experience/current-lessons.md`: AI-specific lessons from this repository.
