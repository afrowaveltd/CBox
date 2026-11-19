# ProveIt — design decisions and research backlog

**Recorded:** 2026-11-18. **State:** preserved conversation intent, not approved implementation contracts. The design is deliberately allowed to change after hands-on CConsole testing.

## Agreed direction / strong preferences

| Subject | Current direction | Why |
| --- | --- | --- |
| Purpose | Separate ProveIt proposal inside CBox | Reusable test infrastructure with a simple author experience |
| Hierarchy | Assertion -> Test -> Block -> Tree/Suite | Clear parent/child responsibilities |
| Failure semantics | A failed assert cannot be undone by a later PASS | Reliable monotonic failure aggregation |
| Diagnostic granularity | **Exactly one message record per failed assertion**; no message for a passed assertion | Small memory use and precise errors |
| Test summary | Read/aggregate all child assertion failures | A test may have many failing assertions |
| Data storage | Fixed-size independently allocated blocks, no relocation of existing data blocks | Stable addresses and straightforward indexing |
| Candidate capacity | 1,000 records per block, 4,096 block pointers -> 4,096,000 records per shared collection | Generous practical ceiling without up-front data allocation |
| Failure text storage | Normally write failure messages to append-only `test.dat` | Avoid long text payloads in RAM |
| In-RAM diagnostic reference | Byte offset + length, absent for PASS | Direct retrieval when needed |
| Timings | Measure elapsed duration natively, display as s/ms/ns | Useful for test diagnostics and future performance investigations |
| Reporters | Basic, Lite (**default**), Full (optional CConsole) | Independent test core, progressively richer output |
| Run control | Later selection, repeats, profiles, interactive run/skip | One framework for small and large codebases |
| Early fail stop | Proposed 100 failed test cases; `--force-complete` override | Avoid drowning in derivative failures |
| Future output | Optional AJIS and XML exporters | Reuse structured results |
| Workflow | Learn via CConsole first, then design/revise ProveIt | First version is a learning tool; later versions improve architecture |

## Open decisions before coding

1. **Naming:** exported `cbox_proveit_*` names, `ProveIt` marketing name, and any historical `CTest` terminology; avoid confusion with CMake's CTest.
2. **Identity:** `TestId\0Message\0` as originally sketched, or `AssertId\0Message\0` / composite IDs to identify multiple failed asserts within one test.
3. **Status types:** bool flag for the simplest fail aggregation vs full test states (NOT_RUN, PASS, FAIL, SKIPPED, INFRA_ERROR); status storage per assertion and/or case.
4. **API generics:** adapter callback signature, explicit typed helper functions, `void *` + type descriptors, and optional `_Generic` wrappers; input/expected/actual ownership and lifetimes.
5. **Ownership:** who allocates parent objects, assertion blocks, and the one shared directory; how to add/remove elements without dangling parent links.
6. **Sizing:** actual `sizeof` of records, block allocation overhead, memory target, 4096 pointer directory cost, behavior for many tiny tests, and out-of-memory response.
7. **Overflow:** whether 4,096,000 is a first-version hard limit or new pointer-directory tiers are added. The latter was discussed but intentionally deferred.
8. **Persistence:** create/truncate `test.dat` per run or lazily at first failed assert; scratch-file path, cleanup, retention, concurrent runs, crash/interrupted-run behavior.
9. **Message reference:** whether offset points to record start or message payload; how to represent no message, empty message, partial write, and variable-length IDs.
10. **File I/O:** 64-bit offsets, buffering, UTF-8 validation policy, text length bounds, no embedded U+0000, and recovery from disk errors.
11. **Timing contract:** monotonic clock adapters, resolution limitations, setup/teardown inclusion, and whether per-assert timers are worth the overhead.
12. **Limit policy:** does the 100-failure stop count failed test cases (current proposal) or individual asserts? exact CLI spelling/configuration; stop behavior with parallel tests.
13. **Selectors and profiles:** stable IDs, wildcards/tags, manual interactive run/skip, CLI/noninteractive behavior and saved format.
14. **Export compatibility:** optional AJIS and XML schemas, escaping, lifetime of referenced disk logs, whether an export retains every assertion or only failures.
15. **DoesItFail relationship:** keep independent as-is, compare learning outcomes, eventually integrate or replace? No migration decision has been made.

## Suggested learning experiments (not an implementation commitment)

- Make a minimal `bool *` / context-and-callback failure example; demonstrate how an assertion's failure reaches its parent.
- Draw three pointer layers, then implement a 1,000-record block directory in isolation. Validate index 0, 999, 1000, and the last valid index.
- Test stable item addresses while appending blocks. Test allocation failure and cleanup.
- Experiment with writing `identifier\0message\0` as raw bytes, storing byte offset/length, and reading the same text back (including non-ASCII UTF-8).
- Simulate a test with 100 assertions of which several fail; verify *one record per failing assertion* and a FAILED parent case.
- Benchmark small/medium/large cases on SSD and HDD before deciding whether any cache or different block size is necessary.
- Check differences between Basic/Lite/Full reporting while asserting that test results themselves remain identical.

## Scope guard

Only **documentation** exists in this folder as of this note. Nothing here updates the current CBox `make test`, `make test-framework`, CConsole code, or the separate DoesItFail repository.


## 2026-11-19 — composable steps, assets and scheduling

This addendum records the latest discussion; full reasoning, examples and open problems are in [BUILDER_RESOURCES_AND_PARALLELISM.md](BUILDER_RESOURCES_AND_PARALLELISM.md). No implementation or stable API is implied.

### Strongly preferred decisions

| Topic | 2026-11-19 direction |
| --- | --- |
| Lifetime owners | \`suite_data\`, \`scope_data\`, \`test_data\`; owner controls cleanup; test never frees borrowed suite/scope resources |
| Test independence | **Per-test init always** guarantees a known starting state. Reset restores working position/defaults; it cannot repair overwritten bytes |
| Cleanup | Always perform owner-appropriate cleanup even after test FAIL; essential resource preparation failure produces ERROR and blocks affected work |
| Persistent fixtures | **Opt-in \`persistent_asset\`**, owned by Suite, immutable to tests, survives normal cleanup; plain \`asset\` is temporary |
| Persistent repair | Explicit \`suite_asset_repair(suite, asset)\` and \`suite_asset_repair_all(suite)\` |
| Bulk repair | **Fail-fast**, abort at the first repair failure; incomplete repair leaves suite unready |
| Test definition | \`test_builder()\` with \`add_step()\`, typed payloads/callbacks, flexible \`steps[]\`, **no enforced semantic step ordering** |
| Step names | Unique within their test; allow selected checks and future named jump targets |
| Preconditions | Optional checks attached to a step and optional explicit repeated \`PRECONDITIONS_CHECK\` steps |
| Input origins | \`USER_INPUT\`, \`FILE_INPUT\`, \`FUNCTION\`, with captured inputs reused in validation and real execution |
| Mock validation | Validate each scenario with expected-success mock before the real tested function; track \`validated\` separately from \`tested\` |
| Flow control | Named \`jump_to\`, \`jump_while\`, \`jump_for\` proposed; zero-pass loops can be composed with an initial skip-to-condition jump |
| Loop safety | No fixed \`MAX_LOOPS\`; optional per-jump repetition watchdog, potentially a future timeout |
| Parallel execution | Parallelize **independent noninteractive Scopes**; initially keep tests within one Scope sequential |
| Mutable data | Never share a writable suite asset across scopes. Sequential use may borrow a correctly reset instance; parallel workers need independent mutable clones/working state |
| Interactive scheduling | **Any Scope containing interactive input is excluded from parallel processing and run sequentially *after* all noninteractive Scopes complete** |
| Concurrent reporting | Never interleave raw worker assertion output; publish complete test reports on completion or after the suite, with independent progress reporting |

### To resolve later

- Relation of the earlier **Test Block** grouping to new **Scope**, and the final public API names.
- Exact \`Step\` tagged-union layout, callback/result types and skip/continue semantics.
- Correct mock validation when steps mutate data or use jumps.
- Targeted step execution with explicit prerequisite state; named-jump counting, iteration semantics and optional timeout.
- Asset presence versus fast/full integrity verification, reconstruction and safe publishing.
- Deep-copy/clone strategy for mutable files and pointers; worker scheduling/diagnostic-log synchronization.
- Whether the older **100-failed-tests stop** proposal is desirable in a runner intended to continue independent tests; no new default is decided.

**Scope guard:** the 2026-11-19 notes only extend the design. No CBox/CConsole/DoesItFail implementation, tests, or build targets were changed.
