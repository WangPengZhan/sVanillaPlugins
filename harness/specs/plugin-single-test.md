# Per-plugin SingleTest

Date: 2026-09-27

## Scope and acceptance

- Preserve the existing BiliBili SingleTest and other user edits.
- Add `<PluginName>BusinessTesting.SingleTest` to each remaining plugin's
  `test/plugin/test_plugin.cpp`, following BiliBili's explicit initialization,
  local editable URL/download configuration, and RAII cleanup pattern.
- Business plugins reuse `runCase` without loading the JSON case list. Set
  downloader creation, Finished status, and output-file expectations explicitly.
  An empty expectedViews list allows replacing the URL without stale IDs.
- Reuse sample URLs from existing cases. XHS's sample token is a placeholder
  and must be replaced with a valid shared note URL before live execution.
- TemplatePlugin has no media implementation: its SingleTest verifies rejected
  URL, empty views, and null downloader instead of pretending to download.
- Do not change production code, CMake registration, or existing flow helpers.

## Verification

Format only changed test files using the existing formatting script's helper.
Run the configure/build/test harness via validate in `out/harness`; use
`GTEST_FILTER=*BusinessTesting.SingleTest` for focused execution. Live business
tests require network access, aria2/FFmpeg, and any platform credentials. Run
from the executable directory with `--gtest_filter=*BusinessTesting.SingleTest`
after replacing expired or placeholder URLs. Compilation or environment blockers
must be reported separately from successful live downloads.

## Result

- `python scripts/harness.py --action validate --enable-test --build-dir out/harness`:
  configure passed; build initially blocked by MSBuild FileTracker E_ACCESSDENIED
  in the sandbox, so the test phase was not reached.
- `python scripts/harness.py --action build --enable-test --build-dir out/harness`:
  passed after retrying outside the sandbox; all plugin test executables built.
- Each of the nine plugin test executables was run with `--gtest_list_tests
  --gtest_filter=*BusinessTesting.SingleTest`: all nine entries discovered.
- With `GTEST_FILTER=TemplatePluginBusinessTesting.SingleTest`,
  `ctest --test-dir out/harness --build-config Debug --output-on-failure -R
  '^TemplatePlugin_plugin_test$'`: 1/1 passed.
- Root and TemplatePlugin `git diff --check`: passed.
- Live downloads were not executed: platform access/credentials and valid URLs
  are required, and the XHS sample explicitly contains a placeholder token.
