# CBox architecture

**Status:** initial design foundation. Technical decisions below distinguish **agreed principles** from **unresolved API details**.

## Vision

Build reusable, small C17 libraries that share behavioral goals, terminology, and test cases with [Afrowave Toolbox](https://github.com/afrowaveltd/Toolbox), while respecting C-specific constraints: explicit pointer validity, object lifetime, ownership, error reporting, and ABI boundaries.

The first consumer is [MemView](https://github.com/afrowaveltd/MemView). Learning and actual product use reinforce one another.

## Module hierarchy

```text
Applications (e.g., MemView)
  |                       |
  v                       v
when_it_fails ----------> essentials
                             |
                             v
                    C17 standard library
```

- `essentials`: lowest-level types and small, reusable contracts; no dependency on `when_it_fails`.
- `when_it_fails`: additional failure handling, diagnostics, and policies; it may consume `essentials`.
- Future independent modules may depend on `essentials`, but their dependency edges must be intentional and documented.
- Separate module headers, implementations, tests, and documentation are planned; start with a central Makefile and split the build only if useful.

## What compatibility means

There are two kinds of compatibility:

1. **Behavioral:** given a specified input, each implementation produces an equivalent status, error condition, or parsed result.
2. **Binary / ABI:** object layouts, pointer sizes, calling conventions, or link formats match. We do **not** promise this across C and .NET (or Rust, JavaScript, etc.).

Reusable test fixtures should describe behavior rather than assume particular memory layouts. Fixtures and their schemas live under `specifications/` once defined and reviewed.

## First API candidates (not yet implemented)

Existing .NET reference types live in Toolbox `Essentials/Results/`: `Result.cs`, `Response.cs`, `ResponseOfT.cs`.

Candidate C concepts:

- **Result:** operation outcome without a data payload.
- **Response:** operation outcome with an optional payload (possibly a typed variant).
- A status model and failure description shared as far as practical with Toolbox.
- A higher-level `when_it_fails` module, **not** a dependency of Essentials.

### Open design decisions

Before adding public C headers, settle:

- Is the first status model strictly `OK/FAIL` or does it reserve richer states found in Toolbox (warnings, invalid input, not found)?
- Is a failure description a borrowed `const char *`, an owned copy, a fixed-size buffer, or a separate message type?
- Do Responses use `void *` plus a size and ownership policy, explicitly typed variants, or generated typed wrappers?
- Who allocates and frees any payload, and on which side of the API boundary?
- What happens with a successful result that legitimately carries no data?
- How are malformed or contradictory status/payload combinations prevented?
- Which names are part of the stable public ABI, and which remain internal?
- What are the minimum allocation and portability requirements?

Do not freeze a public API until tests describe its behavior.

## Public C conventions

- C17 language baseline; C99/GNU-specific deviations require a deliberate decision.
- Filenames, modules, and function names use `snake_case`.
- Exported symbols must start with `cbox_`; do not claim C namespaces exist.
- Public headers will be in `<cbox/essentials/...>` and `<cbox/when_it_fails/...>`.
- Internal implementation details remain in `src/`.
- Use Allman braces, four spaces, C17 warnings, and clear English documentation.
- Memory ownership and lifetime are explicit in every API contract.
- Avoid dependencies on a UI framework, OS-specific APIs, or heap allocation in Essentials unless justified.

## Testing strategy

1. Test constructors, success/failure cases, invalid inputs, boundary conditions.
2. Test ownership and cleanup contracts (including repeated cleanup if the API permits it).
3. Run compiler warnings and diagnostic builds.
4. Use MemView to exercise the public API in real application code.
5. Define language-neutral scenarios only after agreeing on common semantics.

The initial `make test` only validates toolchain setup; it does **not** test any library yet.

## Independent development-time testing framework

[DoesItFail](https://github.com/afrowaveltd/DoesItFail) is an **external, standalone C17 library** providing a test-case registry, checks, structured results, and optional reporters. It is not the CBox `when_it_fails` module, and it does not replace .NET test tooling. A production build of `essentials` must never link DoesItFail. Consumer-owned test cases live in CBox itself; only test executables may link the external library.

The initial opt-in integration is `make test-framework`, leaving `make test` and learner-owned Result tests unchanged. For ownership, environment and repeatable module setup see [integration instructions](DOES_IT_FAIL_INTEGRATION.md). For exploratory AJIS/AjisFS performance and disk design discussions see [research notes](EXPERIMENTAL_STORAGE_NOTES.md), which are **not** promises of current CBox functionality.
