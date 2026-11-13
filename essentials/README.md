# Essentials

The lowest-level CBox module. Intended to mirror the **purpose** of [Toolbox Essentials](https://github.com/afrowaveltd/Toolbox/tree/master/Essentials), not its .NET internals.

## Initial scope

- A small `Result` type: success or failure without payload.
- A `Response` type: a status and optional data, with an explicit ownership contract.
- Only essential shared status, issue, or guard concepts when a real need arises.

**No public API exists yet.** In particular, this README does not promise any finalized struct layout, enum, function name, or allocator policy.

## Dependency boundary

Essentials must not depend on `when_it_fails`, UI frameworks, or unnecessary third-party libraries.

## Planned layout

- `include/cbox/essentials/`: public C headers
- `src/`: implementation files
- `tests/`: independent unit tests
- `docs/`: examples and per-function reference documentation

The `.gitkeep` placeholders allow these empty directories to survive a Git clone.

Before writing code, see [Architecture](../Docs/ARCHITECTURE.md) and [API documentation rules](../Docs/API_DOCUMENTATION.md).
