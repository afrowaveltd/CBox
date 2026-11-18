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
