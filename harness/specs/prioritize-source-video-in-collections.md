# Prioritize Source Video In Collections

Date: 2026-07-20

## Context

Some video URLs resolve to the complete collection containing that video. The
frontend displays results in the order returned by the plugin, so the video the
user opened can otherwise be buried later in the collection.

## Goal

When a URL identifies both a specific video and a collection, return that
specific video as the first item while preserving the relative order of all
other items.

## Scope

- BiliBili video, bangumi episode, and cheese episode URLs that return multiple
  related items.
- YouTube video URLs containing a playlist ID.
- Pure collection URLs retain the service-provided order.

## Acceptance Criteria

- The source video moves to index zero when it exists in the returned list.
- Other collection items keep their original relative order.
- Missing source IDs and empty/single-item results remain unchanged.
- Deterministic unit tests cover the ordering behavior.

## Harness Plan

Build and run the BiliBili and YouTube unit-test targets from `out/harness`.
