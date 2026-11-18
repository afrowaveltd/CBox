# ProveIt — working architecture notes

**Recorded:** 2026-11-18  
**Maturity:** discussion / sketch, **not a finalized specification or existing implementation**.  
**Language target:** C17. Source code and public identifiers are intentionally not frozen.

This is a record of the design conversation so it can be resumed without reconstructing decisions. Examples are illustrative, not promises or ready-to-compile API contracts.

## 1. Intended user experience

The caller should be able to add/remove/group assertions and tests easily, run one test or a large library's suite through one interface, and receive clear failure details. Generic-feeling use similar to C# / xUnit is the **ergonomic goal**; C has no reflection or ordinary C#-style generics, so the mechanism will be designed explicitly.

Early mental model: supply **test inputs**, **expected output**, and **a callback/function to exercise**. A minimal adapter/callback can translate typed C parameters to a common runner interface. Alternatives for safe typed assertions include typed functions or later C11/C17 `_Generic` conveniences. Avoid assuming arbitrary function pointers can portably be cast to `void *`.

These points are *questions for the C-learning implementation*, not an API decision. Prefer simple user-written tests even if the core contains additional plumbing.

## 2. Responsibility hierarchy

```text
Test Tree / Suite                  coordinates the run and summaries
  |
  +-- Test Block                   owns/groups related test cases
        |
        +-- Test Case              runs a function; aggregates its assertions
              |
              +-- Assertion        performs exactly one check
              +-- Assertion        performs exactly one check
              +-- Assertion        performs exactly one check
```

- **Assertion:** one comparison/check and its result; on failure, exactly one diagnostic message record.
- **Test Case:** a collection of assertions. It evaluates to FAILED if one or more child assertions fail; it can enumerate **all** its failing assertions. A test does not fabricate or concatenate their diagnostic messages.
- **Test Block:** groups related cases by function, feature, or module, then aggregates results.
- **Test Tree/Suite:** groups blocks, chooses execution scope, accumulates global statistics, and coordinates reporting.

An assertion may carry a parent reference and the identity/inputs/outputs pertinent to its check. Tests may hold a list/collection of assertions; blocks hold tests; suite holds blocks. Parent links, logical IDs, and memory ownership must be drawn explicitly before implementing.

The *failure flag* can be monotonic: initialized to success, and an assert failure can flip it to failure; later success must **never reset** it. A `set_fail` callback is one possible place to update status and enqueue a diagnostic message. In C, a callback's state/context must be passed explicitly, perhaps as a pointer to a context struct. Parent failure can likewise be aggregated from children.

**Distinguish not-run, skipped, failed assertions, and framework/infrastructure errors.** A zero failure counter or `failed == false` alone does not prove execution happened.

### Example

```text
Suite CBox
  Block Blackboard / Output
    Test WriteLiteral              4 assertions
      #1 PASS
      #2 FAIL  -> "expected written=5, actual=4"
      #3 PASS
      #4 FAIL  -> "expected error=NONE, actual=IO"

Result: test WriteLiteral failed; 2 distinct assert failures.
Diagnostics belong to assertions, not to the containing test.
```

## 3. Small in-memory records and stable addresses

Keep the basic live/result record small: stable ID or name reference, PASS/FAIL or richer status, elapsed duration, parent/index as necessary, and **message offset + length only when an assertion failed**. The exact struct layout, field sizes, and whether definition/result are distinct types remain to be decided.

The proposed **segmented array** avoids reallocating populated data:

- A shared directory with **4,096 pointers** (about **32 KiB** on a machine with 8-byte pointers).
- Each non-null entry points to a separately allocated block of **1,000 assertion records**.
- Allocate blocks **only as they fill**. Do not preallocate millions of records.
- Nominal capacity: `4096 * 1000 = 4,096,000` assertions **per shared collection**, not automatically per test case.
- Global logical index `i` maps to `blocks[i / 1000][i % 1000]`. This is a bounded number of direct lookups.
- When adding a new data block, existing blocks do not move, and retained pointers into those still-live blocks remain valid.
- If the theoretical limit is ever reached, adding another pointer-directory level/group is an idea for later, **not** required in the first version.

A 4,096-entry table embedded in **every small test** would waste memory; explore suite-level shared storage, smaller directories, or separate collections as appropriate. Blocks have an allocation cost of `1000 * sizeof(assert_record)`, even if partially filled. Test this with actual struct sizes and allocations rather than assuming a fixed memory footprint.

Deleting/reordering assertions, recycling slots, stability of parent references, cleanup, and OOM handling need explicit contracts. A released block invalidates pointers to its elements. Even with stable addresses, ownership and lifetime matter.

**Memory target:** avoid unnecessary growth toward 1 GiB; an upper discussion target was about **500 MiB**, preferably substantially less even for very large suites. This is an aspiration, not a benchmark result.

## 4. Diagnostics: one failed assert = one on-disk record

**No diagnostic message for successful assertions.** Messages explain **which check failed**, with expected versus actual values where meaningful. The test simply finds the failures among its assertions.

Proposed append-only `test.dat` record format:

```text
<identifier UTF-8 bytes> \0 <message UTF-8 bytes> \0
```

Here `\0` means **one NUL byte** (not a slash, not the literal characters backslash and zero). A text value must not contain embedded U+0000. UTF-8 is the proposed Unicode encoding; ASCII is its compatible subset.

The original sketch used **Test ID** as the record identifier; since multiple assertions can fail inside one test, **assertion ID or composite Test ID + Assertion ID** may be preferable. This **has not been settled**.

Store in RAM numeric **byte offset** and **byte length** for the diagnostic message, e.g., candidates `uint64_t message_offset` and `uint32_t message_length`. These are numbers, **not pointers** and cannot hold C `NULL`. Define one explicit absent-message sentinel/rule, for example length zero (with a policy for empty diagnostic messages). Clarify whether offsets start at record ID or message body, and document the choice once the binary contract is established.

On failure:
1. Mark that assertion failed **even if subsequent diagnostic I/O fails**.
2. Append its one message to `test.dat`, recording exact offset and byte count.
3. Keep only the small file reference in RAM; reread the UTF-8 message on demand for a detailed report/export.

The user's preference is to make **disk-backed failure text the normal behavior**, not an elaborate memory cache/spill subsystem. With no failures, no message needs to be written. Whether `test.dat` is truncated at test-run start or created lazily on the first failure is still open; either must avoid stale references. Keep writes buffered/sequential rather than flushing after every tiny record.

Storage failure (cannot open, short write, full disk, read failure) is a **framework diagnostic problem**, not evidence that a tested function passed or failed. Provide a graceful fallback/error report. Do not overwrite another concurrent test run's `test.dat`; define per-run paths or exclusive ownership later. A result's file offsets are only useful for as long as the corresponding file remains available.

**File portability:** use an approach suitable for >2 GiB files where supported, and explicitly define byte offsets, encoding, and the text framing rather than assuming `ftell` is universally 64-bit.

## 5. Timing

Timing is a **native capability**, particularly at the test-case/run level. Candidate stored form: one integer elapsed nanosecond count (`uint64_t elapsed_ns`); reporters can format the same value as **seconds, milliseconds, or nanoseconds**. Nanosecond units do **not** guarantee nanosecond clock accuracy.

Choose a monotonic elapsed-time clock suitable for each supported platform (e.g., POSIX `clock_gettime(CLOCK_MONOTONIC)`, Windows `QueryPerformanceCounter`) behind a small adapter. Define whether setup/teardown or reporting overhead is included. Fine-grained per-assert timing is optional: instrumentation may dominate tiny assertions.

## 6. Execution policies, selection, and profiles

Plan eventual filtering by suite/block/test, repeated runs, test profiles, and optional **interactive prompts** before a test ("run / skip"). These controls belong in the runner, not inside a pure test assertion.

A proposed default safety stop is **100 failed test cases**, with `--force-complete` to continue beyond the threshold. The distinction between failed tests and failed assertions matters: one test with many failed assertions counts as **one** failed test case for this proposal. Let a currently running test finish, then stop scheduling more tests. Remaining tests are **not run**, not PASS. Exact flag spelling, threshold configuration, and failure granularity are still design-stage.

For repeats, distinguish **test definition** from **each execution's result** so repeated failures, elapsed times, and intermittent failures are not overwritten.

## 7. Reporting and export

Three console formats:

| Mode | Intended output | Dependencies |
| --- | --- | --- |
| **Basic** | Plain, readable line-oriented output, friendly to CI/logs | C standard/POSIX/Windows minimal environment |
| **Lite** (**default proposal**) | Light terminal colors and formatting; no full tables | Tiny self-contained ANSI/terminal handling, **not CConsole** |
| **Full** | More elaborate tables, colors, themed output | **Optional** CConsole/Blackboard integration |

All three consume the **same structured result model**. The test core must remain independent of Blackboard/CConsole. Lite must gracefully fall back when ANSI output is inappropriate (redirects, no color, unsupported terminal).

**Future optional exports:** AJIS and XML. Exporters should read result data/diagnostics without entangling the test evaluator with text formatting. No schema or binding to an AJIS implementation is decided.

## 8. Boundaries and learning sequence

- Start with CConsole's **small handwritten test publisher**. It is intentionally independent and provides the learning experience needed to design ProveIt more confidently.
- No hidden production code or frozen API yet. Draw pointer ownership and function-call flow; build minimal experiments; then verify behavior under C17 with warnings and tests.
- Do not assume a generic test framework eliminates mocking: replacing file/network/database dependencies is a separate concern, though callbacks can help.
- Distinguish desired architecture from current repository reality. The existing **DoesItFail** repository and its documented CBox opt-in linkage remain untouched. How ProveIt relates to it is undecided.
