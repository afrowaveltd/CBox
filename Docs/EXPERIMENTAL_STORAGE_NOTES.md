# Exploratory AJIS / AjisFS storage notes

**Status: discussion notes and experiment candidates, not implemented CBox features and not guaranteed AJIS contracts.** Preserved here so useful details do not get lost when focus returns to C teaching.

## Standards and resources

- C17 is the portable baseline. Consider C99 selectively for genuinely useful legacy targets (e.g., lightweight AjisFS tools on an old NAS), never as an unsupported blanket promise.
- Budget RAM, CPU, threads, and disk space explicitly. Measure behavior on HDD, SSD and low-resource systems; do not assume C code is automatically faster or memory-efficient.
- Memory-saving, balanced, and throughput-oriented strategies should preserve identical logical results. Async I/O and CPU parallelism are independent.

## Object processing and bounded queues

- A producer recognizes complete independent objects and distributes them across bounded worker queues, preferably in round-robin fashion when useful.
- A worker drains its queue, then waits for data on a condition variable. Signal on empty-to-nonempty transitions. Protect queue state and wait conditions with the same synchronization discipline; `cnd_wait` needs a `while` predicate loop to avoid lost/spurious wakeups.
- Make number of workers, queue slots, object byte sizes, and total memory configurable. A limit of 25 objects does **not** by itself cap RAM use.
- Do not start a thread per object. Respect AJIS object boundaries, references/pointers/directives and the logical output order.

## RAM and disk stream spooling

- Use RAM as the fast path; spill buffered input into bounded disk-backed segments if parsing falls behind ingestion.
- Reuse disk segments only after acknowledgment of their last consumer, not merely after an early read.
- A source permanently faster than consumers causes an ever-growing backlog: enforce disk quota, backpressure, or explicit failure rather than silent data loss.
- A virtual block sequence in an ordinary file is **not necessarily physically contiguous on HDD**. Compare real seek behavior.

## AjisFS workspace and commit frontier

- Candidate representation: fixed-size logical blocks linked by `next`, optionally `previous`. Maintain a head and a **persistent committed `last_used` / tail**.
- Append can write the new block, patch the old tail's next reference, and **only then** advance the durable committed tail. Readers stop at the committed tail even when an uncommitted next link remains.
- A failure before commit may leave orphaned blocks. They are not valid data; the allocator may reclaim them only after considering all reachable roots, retained versions, and active readers.
- A working tail in RAM plus periodic durable commit can trade performance for losing up to the last uncommitted batch after a crash.
- Durability requires actual ordering and flushing and protection against torn metadata writes. A mere `fwrite` or updating a RAM variable is not a crash-safe commit.
- Backward links make logical middle insertions/updates convenient once the position is known, but rewriting both neighbors is **not** protected atomically by the append-tail rule.
- During edits, allow nonsequential logical blocks and seeks. On **save/compact**, write a new sequentially preferred representation, then publish it by changing the virtual-FS document mapping only once the new data are durable. Reclaim old storage only after old readers/versions no longer need it.

## Experiments to revisit

1. Flat sequential files vs random linked blocks vs contiguous-preferred linked blocks on real HDD.
2. Per-block vs batch commits, durable tail record update cost, and failure injection after every step.
3. RAM peak, disk spool quota and backpressure under slow consumers.
4. Copy-on-write compacted document publication and recovery after abrupt termination.
5. Block ID format, file-offset width, endian/ABI portability, and large-file behavior on older hardware.
6. Whether backward links or a separate index justify their extra writes.

These notes record ideas, **not measured performance claims**.
