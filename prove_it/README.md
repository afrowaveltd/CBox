# ProveIt — proposed CBox test framework

> **Status (2026-11-18): design notes only.** No implementation, public API, CLI, build integration, or published package is claimed. The design grew out of hands-on C17 tests for Blackboard/CConsole. Implementation should follow deliberate C practice and experiments, not precede them.

**ProveIt** aims to make writing, grouping, selecting, running, and diagnosing C tests as easy as possible while keeping the mechanics understandable: structs, pointers, callbacks, explicit ownership, and measured memory usage. Inspiration includes xUnit's organization, but this is a native C17 design rather than a copy of its API.

## Documentation

- [Architecture and data flow](docs/DESIGN_NOTES.md) — the ideas discussed so far, including assertions, test hierarchy, block allocation, diagnostics in `test.dat`, timing, and output modes.
- [Decisions and open questions](docs/DECISION_LOG.md) — distinctions between agreed intent, examples, and unresolved details.

## Current intent in brief

- **Assertion -> Test -> Test Block -> Test Tree/Suite**: each level owns responsibility for its children and aggregates their results.
- One assertion is one check; **one failing assertion produces one diagnostic record**. Successful checks do not write messages.
- A test fails if **any** of its assertions fails. A failure cannot be reversed by a later successful assertion.
- Favor a small in-memory result (identity/name, status, elapsed time, optional diagnostic reference); save failure text to a sequential on-disk `test.dat` file, accessed by byte offset and length.
- Explore fixed-size **1,000-item blocks**, indexed by a pointer table with **4,096 entries** (nominal capacity **4,096,000 assertions** per shared collection), allocating blocks on demand, **without relocating existing data blocks**.
- Three console reporters: **Basic** (plain text), **Lite** (light ANSI colors/formatting; proposed default), **Full** (optional CConsole/Blackboard rendering). Core must not depend on CConsole.
- Plan selectable tests/blocks, repeat runs, user-defined profiles, optional interactive run/skip prompts, a default fail-stop policy (proposed 100 failed tests, `--force-complete` to override), and future AJIS/XML exports.
- Timestamp/duration measurements should be part of the test-run result; elapsed time represented internally in one unit (candidate: nanoseconds).

## Context and boundaries

The repository currently also documents **[DoesItFail](https://github.com/afrowaveltd/DoesItFail)** as an independent, development-time test framework. **ProveIt is a new CBox proposal; it does not silently replace DoesItFail or change any existing build/test target.** Future coexistence, migration, or consolidation is deliberately undecided.

CConsole's initial handwritten tests remain their own small learning exercise and must not acquire a ProveIt dependency merely because these notes exist.

Folder naming follows the CBox `snake_case` convention: `prove_it/`. The public product name is **ProveIt**. No headers or source files have been created yet.
