# ProveIt — step builder, resources, input and execution scheduling

**Discussion recorded:** 2026-11-19  
**Status:** architecture notes / design intent; **not implemented and not a frozen public C API**.  
**Context:** C17, following the handwritten CConsole/Blackboard test exercises. Read this alongside [DESIGN_NOTES.md](DESIGN_NOTES.md) and [DECISION_LOG.md](DECISION_LOG.md).

This document preserves the next iteration of the ProveIt conversation. It adds a *composable step program* and explicit resource lifetimes to the earlier assertion → test → block/suite model. The relationship between the old **Test Block** term and the newer **Scope** level is not yet frozen. Examples below are illustrative pseudocode, not headers that currently exist.

## 1. Direction and guiding principles

- A small C core can offer a high-level testing experience without reflection, hidden discovery, heavyweight dependencies, or xUnit-style ceremony.
- Tests are **explicit sequences of steps**; the builder defines them and a runner interprets them. More elaborate scenarios are assembled from a small set of instructions.
- The runner must **not require a fixed order** such as “all preparation → all assertions.” A legitimate test may assert, change data, assert again, and repeat.
- Preparation, validation, execution, reporting, and cleanup have different responsibilities. An ordinary failed assertion must not silently prevent independent later tests or skip necessary cleanup.
- Advanced features are optional; a simple test must remain simple.
- Prefer C17 conventions, explicit callbacks, Allman braces, and strong ownership contracts. Object pointers can use \`void *\` where suitable; do **not** interchange arbitrary function pointers with \`void *\` (not portable ISO C).

## 2. Resource owners: Suite → Scope → Test

Three intended ownership/lifetime areas:

| Area | Lifetime | Typical use |
| --- | --- | --- |
| \`suite_data\` | From suite init through suite cleanup | Shared, read-only reference data; suite-owned persistent assets |
| \`scope_data\` | One scope | Working assets used by its tests, isolated from other scopes |
| \`test_data\` | One test invocation | Results, scratch buffers, test-specific mutable data |

A context may expose borrowed pointers to all three areas without owning their allocations. **The level that owns a resource releases it.** A test must not free a suite-owned or scope-owned object. Release in reverse order: test → scope → suite. Partial initialization still requires safe cleanup of whatever was successfully acquired.

Do not put a shared, mutable working asset in the suite for all tests. Mutable data should be small and owned by the relevant scope or test. The suite may own an immutable template or shared read-only reference data.

**Test independence:** each test must begin in an explicitly defined resource state, even if a previous test failed or corrupted its working data. Reusing the same mutable object in sequential mode is allowed only when its baseline is re-established. A passing result of Test A must never be a precondition for Test B.

### Init, reset, cleanup

- \`suite_init()\`: initializes shared suite context and checks/creates required persistent assets before scopes run.
- \`scope_init()\`: prepares that scope's own resources.
- \`resources_init()\` / test init: **required before each test/scenario**, ensuring precisely the baseline that scenario needs. It may reuse an already valid input rather than regenerate it.
- \`resources_reset()\`: restores operational position/default working values (e.g. \`rewind()\`, counters). It **does not implicitly restore overwritten file bytes**.
- \`test_cleanup()\`, \`scope_cleanup()\`, \`suite_cleanup()\`: always release their respective short-lived handles, memory and mutable assets when their owner ends, regardless of the test PASS/FAIL result.

When a file's content was changed, the next scenario must use \`resources_init()\` (or explicit content reconstruction) to guarantee the baseline; \`rewind()\` is not repair. \`init()\` should ideally be idempotent with respect to the requested scenario: requesting an AJIS containing one attachment twice must not append two attachments.

Failure of a **test assertion** means record FAIL and continue according to test execution policy. Failure of **essential resource initialization** means infrastructure ERROR: do not execute affected tests. Other independent scopes/suites may continue. Cleanup attempts still run.

## 3. Persistent assets: explicit, suite-owned, immutable to tests

Persistence is exceptional, so do not require a policy enum on every ordinary asset:

- \`asset\`: ordinary temporary mutable working data, cleaned up by its owner.
- \`persistent_asset\`: explicitly registered with / owned by the **suite**, survives program runs, and is **read-only to scopes and tests**.

Typical example: a generated **20 GiB Markdown input** for terminal-viewer performance tests. Generating it on every run is wasteful. In \`suite_init()\`, find it, verify that it is suitable, or create it; report each phase and progress to the user.

A persistent asset is a reference fixture, **not the file into which an individual test writes**. Tests writing to files must have their own mutable working copy or independently prepared working file. Large files may need a different efficient strategy rather than unconditional deep copying.

A \`const ProveItAsset *\` can expose a read-only descriptor but C \`const\` alone cannot protect a path on disk. Provide read-only access/handles and enforce ownership rules. Closing a test's read handle is fine; deleting or modifying the suite's persistent file is not.

\`suite_cleanup()\` closes/relinquishes handles and RAM but **does not delete persistent files**. Explicit suite-authorized maintenance is separate.

### Validate and repair

The intended public names are deliberately explicit:

~~~c
bool suite_asset_repair(
    ProveItSuite *suite,
    const ProveItAsset *asset
);

bool suite_asset_repair_all(
    ProveItSuite *suite
);
~~~

No \`NULL\` sentinel to mean “all,” and no argument that must be supplied for every ordinary asset.

- \`suite_init()\` must verify asset presence/readiness; exact validation strategy remains a design question.
- A present file may still be corrupted. For small AJIS assets, full validation can be practical; for a 20 GiB file, a fast check (manifest/version/size/selected probes) versus optional full integrity check is worth evaluating.
- A generator/version identity, completed-generation marker and reproducible parameters are candidate manifest fields.
- Repair regenerates a single asset or **all persistent assets of that suite**. Rebuilding via a temporary file and publishing only after verification is a proposed safety measure.
- **Fail fast in \`suite_asset_repair_all()\`: return false on the first failed repair and do not attempt later repairs.** A partially repaired suite is **not ready**. Earlier completed repairs may remain; no global rollback is promised.
- Report why a repair failed (permissions, no space, invalid contents, generator error, etc.). Avoid modifying files outside suite-managed asset locations.
- Suite-owned maintenance (repair/delete) must not run while parallel scopes hold live read handles to the affected asset.

Suggested events: asset found, validating, missing, generating, progress, ready, invalid, repair started/failed/completed. Progress reporting is separate from test output.

## 4. Composable Test Builder / steps[]

A \`test_builder()\` registers steps with \`add_step()\`. A Test Runner executes the resulting \`steps[]\`, normally in insertion order. A step has a **kind**, optional **unique name within its test**, any relevant data/callback, optional preconditions, and an execution result.

Tentative kinds (names provisional):

| Step kind | Meaning |
| --- | --- |
| \`TESTING_VECTOR\` / \`TEST_VECTOR\` | Establish input values and expected outcomes |
| \`PREPARE_DATA\` | Establish/modify working data |
| \`PREPARE_FUNCTION\` | Prepare helpers, mocks or injected dependencies |
| \`TESTED_FUNCTION\` | Call the real function via a typed adapter |
| \`ASSERTION\` | Verify a result, invariant or side effect |
| \`PRECONDITIONS_CHECK\` | Explicitly evaluate prerequisites |
| \`INPUT\` | Acquire test values from a source |
| \`JUMP\`, \`JUMP_WHILE\`, \`JUMP_FOR\` | Future control flow |

No hardcoded step order: \`ASSERTION → PREPARE_DATA → ASSERTION\` is valid when meaningful. A builder can still reject malformed *definitions* (missing callbacks, duplicate step names, invalid destination), but it must not legislate the semantics or ordering of a developer's test.

Use a **tagged union** (\`enum\` + \`union\`) for step-specific payloads and appropriately typed callback pointers. Test vectors are better represented as arrays of structs (one tuple of inputs/expectations per case) than unrelated parallel arrays that may drift out of sync. An expected safe error such as division by zero can be a normal test vector rather than a deliberate process crash.

### Preconditions

Two complementary options are proposed:

1. An optional **per-step precondition callback**, evaluated immediately before that step, protecting it from unavailable data.
2. An explicit, repeatable \`PRECONDITIONS_CHECK\` step, placeable anywhere in \`steps[]\`.

Not meeting a prerequisite must not be confused with the tested function producing a wrong result: distinguish SKIP, infrastructure ERROR and FAIL according to the actual cause. A targeted standalone assertion should not silently run unspecified preparation steps; its prerequisites must be satisfied explicitly.

### Names and selection

Named steps allow future calls conceptually like:

~~~text
run_all_checks(test);
run_check(test, "check_crc");
run_checks(test, ["prepare", "validate basic", "validate fail"]);
run_steps(test, selected_names, count);
~~~

This is illustrative, not valid C syntax for the list literal. Consider distinguishing “run selected assertion checks” from “run arbitrary selected steps.” Selecting a step does not remove its prerequisite contract. Names are unique **within a test**, not necessarily across the entire suite. The builder could resolve names to step indices at build time.

## 5. Jumps, loops and optional watchdogs

The step list is a tiny interpreter: normally advance to the next step, or change the current step index on a jump. Named targets permit composition of \`while\`, \`do…while\`, and \`for\`-like flows without building a separate imperative language.

Conceptual builder operations:

~~~text
jump_to("check_next")
jump_while("check_next", condition_callback)
jump_for("check_next", iterator_callback, condition_callback)
~~~

The callback signatures are not finalized; prospective typed signatures include \`bool (*condition)(const ProveItContext *)\` and \`void (*iterator)(ProveItContext *)\`. An iterator might eventually need a richer return status to distinguish end-of-data and error.

A backward conditional jump after the body behaves like \`do…while\` (one mandatory pass). For a **zero-pass loop**, jump first to the condition step, which may return to the body only if the condition is true. This can recreate a conventional \`for\` without a special block construct. A \`jump_for\` that first advances its iterator must not accidentally skip the first value on this path.

Do **not** impose a fixed global \`MAX_LOOPS\`: millions of valid iterations are possible in flag sweeps and benchmarks. Offer optional, caller-chosen limits **per jump**, plus potentially a future timeout/watchdog. \`uint64_t\` is a better portable fixed-width limit type than \`unsigned long\` (32-bit on common 64-bit Windows ABIs). Define whether a limit counts *taken jumps* or full passes; taken jumps is the present proposal.

A jump-count watchdog cannot interrupt a callback that hangs internally. If hard timeouts are later required, investigate isolation/interrupt semantics separately.

Control flow **must not bypass lifecycle cleanup**, which belongs outside the optional \`steps[]\` program.

## 6. Inputs and interactive tests

Input origin is independent of the test consuming it. Proposed source enum:

~~~c
typedef enum
{
    INPUT_USER,
    INPUT_FILE,
    INPUT_FUNCTION
} ProveItInputSource;
~~~

- **USER_INPUT:** prompt for input at runtime.
- **FILE_INPUT:** read a developer-specified file (e.g. \`test_data.ajis\`).
- **FUNCTION:** call a data generator with explicit parameters (e.g. AJIS block count).

Acquire/validate an input **once** and use the same captured test vector for mock validation and real execution. Interactive values can optionally be saved for reproduction. In unattended runs, explicit selection/skip/error behavior must prevent hanging on a prompt.

**Scheduling decision (2026-11-19): if any test in a Scope requires interaction, exclude that entire Scope from the parallel queue and execute it only after all noninteractive parallel Scopes have finished.** Interactive Scopes then run **sequentially**, one at a time. This prevents racing prompts and stdout interleaving.

## 7. “Test the test” before testing the function

Each real test scenario should first verify its own preparation and expectations:

1. Required \`resources_init()\` establishes the explicit baseline.
2. Prechecks ensure required input/resources exist.
3. **Validation phase:** replace the real tested invocation with a known controlled mock, using the **same vector and assertions**; verify that they accept the expected correct result.
4. Restore state with \`resources_reset()\` where sufficient or scenario init where content must be rebuilt.
5. **Execution phase:** invoke the real function and run assertions.
6. Record PASS/FAIL/ERROR/SKIP and always perform owner-appropriate cleanup.

Track \`validated\` and \`tested\` independently; “validated” means the test harness successfully checked the scenario against the controlled expected-success behavior, not just that a validation function happened to run. A future negative validation (verify that assertions reject a bad result) is optional.

The placement and semantics of mock substitution within arbitrary steps/jumps still need a concrete design; a simplistic replay of side-effecting prep steps may require extra resets. Do not claim this is already implemented.

## 8. Parallel Scope execution and mutable copying

The suite first prepares/validates shared immutable data. Then it may run **multiple independent, noninteractive Scopes concurrently** using a configurable number of workers. Tests **within a Scope** remain sequential initially.

- **Parallel OFF:** a scope/test may use a pointer to an existing mutable working instance, but only after guaranteeing the test's specified baseline.
- **Parallel ON:** each concurrently running scope/test must receive its **own independent mutable instance**, cloned/prepared rather than pointed at shared writable state.
- **Persistent suite assets** remain immutable reference inputs in both modes, so their on-disk content is not cloned just to permit concurrent readers. Use independent reader handles/positions as needed.
- A C struct assignment containing pointer members is **shallow copy**; it does not clone pointed-to storage. Provide an appropriate \`asset_clone()\`/deep-copy or fresh-init procedure for mutable assets.
- Separate memory does not prevent collisions on the same mutable file path: also isolate paths/handles where writes are involved.
- Resource-heavy performance measurements may need parallelism disabled to avoid disk/CPU contention corrupting comparability.

Simple scheduling sketch:

~~~text
suite_init() + persistent asset validation
    |
    +-- queue noninteractive scopes --> worker pool (max_parallel_scopes)
    |       scope_init -> tests -> scope_cleanup
    |
    +-- wait until all noninteractive scopes finish
    |
    +-- run interactive scopes one by one
    |       scope_init -> tests/prompts -> scope_cleanup
    |
suite_cleanup()
~~~

Failures in one *test* do not imply other independent tests must stop. A broken prerequisite prevents only dependent work. Optional global fail-stop policies from the earlier design notes are **not yet reconciled** with this more flexible execution model.

## 9. Reporter and diagnostics: no interleaved assertions

Workers do not write their assertions directly to the shared terminal. Emit structured events/results to a reporter, isolate per-test output, and present a **whole completed test** as an indivisible readable unit.

Two report timing modes are useful:

- **On completion:** publish each completed test atomically, in completion order.
- **At end:** collect and display in deterministic suite/scope/test order after execution.

A lightweight concurrent status/progress display is allowed without interleaving assertion logs. Long tests may spool logs to disk rather than buffering millions of assertion messages in RAM. Integrate with the earlier \`test.dat\` proposal: record **failed assertion diagnostics**, and do not let parallel runs collide on a shared file; use per-run ownership and coordinated writes or partitioned logs.

No direct dependency from the test core to CConsole/Blackboard. Earlier proposals for Basic, Lite and optional Full reporters remain valid; parallel reporting is a coordination concern in front of these renderers.

## 10. Responsibilities at a glance

| Component | Owns |
| --- | --- |
| **Test Builder** | Declares/validates definitions, names, kinds, callbacks, vectors, flow targets |
| **Test Runner** | Executes steps, validation/mock and real phases, gathers assertions and step outcomes |
| **Scope Runner** | Initializes/releases scope state; sequential test execution inside its scope |
| **Suite Runner** | Suite init/cleanup, persistent asset readiness/repair, parallel noninteractive scheduling, deferred interactive scopes |
| **Reporter** | Formats and emits isolated results/progress; does not determine test outcomes |

## 11. Decisions versus open questions

**Strong direction / design intent from this discussion**

- Explicit owner lifetimes: \`suite_data\`, \`scope_data\`, \`test_data\`.
- Required per-test init; optional reset; reliable cleanup regardless of PASS/FAIL.
- Persistent assets are exceptional, suite-owned, immutable to tests, and survive normal cleanup.
- Explicit \`suite_asset_repair(suite, asset)\` and \`suite_asset_repair_all(suite)\`; bulk repair fails immediately on first error.
- Flexible, named steps with no required semantic ordering; optional preconditions and conditional jumps.
- USER / FILE / FUNCTION input sources; interactive scopes deferred **until after all parallel scopes finish**.
- Parallelize independent scopes (not tests within a scope initially); never share writable working state.
- Aggregate each test's assertions into a coherent report unit rather than interleave worker output.

**Open / requires experiment before freezing headers**

1. Exact naming and whether newer Scope replaces or augments earlier Test Block.
2. Step payload layout, callback signatures, errors, semantic treatment of a failed assertion followed by later steps, and runner step result types.
3. Validation/mock replay with arbitrary step ordering and side effects, including saved interactive input.
4. Jump construction API, per-jump repeat counter lifetime, nested loops, timeout and cancellation behavior.
5. How targeted \`run_check(s)\` supplies explicit prerequisites, and what “selection” means for a partially executed test.
6. Input function argument passing without erasing function-pointer types.
7. Asset catalog/manifest format, fast/full integrity policies, atomic file replacement and multi-process coordination.
8. Deep-copy contracts for scopes/tests and cost-aware strategies for very large files.
9. Reporter worker queue, disk-spooled failure diagnostics, stable ordering and memory limits.
10. Interaction between optional global failure-stop policy, scope-level initialization errors, and parallel scheduling.

## 12. Implementation discipline

This is **documentation**, not a request to change the current CConsole tests, CBox build targets or separate DoesItFail repository. Start with the handwritten C tests, understand the mechanics, then implement small verified layers of ProveIt. Keep the default path usable without requiring threads, interactivity, persistent fixtures, flow control, or a complex reporter.
