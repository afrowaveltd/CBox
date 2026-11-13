# CBox

**CBox** is the native C member of the Afrowave library family, developed alongside [Afrowave Toolbox](https://github.com/afrowaveltd/Toolbox).

The goal is a collection of **small, reusable, well-documented C libraries** with portable behavior, explicit memory ownership, minimal dependencies, and meaningful automated tests. The first consumer will be [MemView](https://github.com/afrowaveltd/MemView), our hands-on C learning project.

**Current status: repository foundation only.** No public Result, Response, or WhenItFails API has been finalized or implemented. The included smoke test verifies the C17 compiler/build setup, **not library functionality**.

## Modules

| Module | Responsibility | Dependencies | Status |
| --- | --- | --- | --- |
| [essentials](essentials/README.md) | Fundamental shared types and contracts, beginning with Result and Response | C standard library only (initial goal) | Design pending |
| [when_it_fails](when_it_fails/README.md) | Higher-level diagnostics, errors, and failure handling | essentials | Reserved for later |

Folder names use `snake_case`; exported C identifiers will use an explicit `cbox_` prefix to avoid collisions with application and third-party symbols.

## Repository layout

```text
CBox/
├── essentials/
│   ├── include/cbox/essentials/
│   ├── src/
│   ├── tests/
│   └── docs/
├── when_it_fails/
│   ├── include/cbox/when_it_fails/
│   ├── src/
│   ├── tests/
│   └── docs/
├── specifications/
│   ├── essentials/
│   └── when_it_fails/
├── tests/
│   └── smoke_test.c
├── examples/
├── Docs/
│   ├── ARCHITECTURE.md
│   ├── CONTRIBUTING.md
│   ├── API_DOCUMENTATION.md
│   └── ROADMAP.md
├── .clang-format
├── .editorconfig
├── .gitattributes
├── .gitignore
├── Makefile
└── LICENSE
```

Empty future-source directories are tracked with `.gitkeep` files. **Git does not track empty directories by itself.** Remove a placeholder once real files are added.

## Requirements and initial verification

- A C17-capable compiler (GCC or Clang)
- GNU Make for the included Makefile
- Git for version control

On Debian:

```bash
sudo apt install build-essential gdb
git clone https://github.com/afrowaveltd/CBox.git
cd CBox
make help
make test
```

`make test` currently compiles and runs only `tests/smoke_test.c`. It deliberately does **not** imply that Result, Response, or other modules exist.

```bash
make clean
```

Build output is placed in `build/` and excluded from Git. The project uses C17, warning flags, 4-space indentation, and **Allman** brace formatting (see `.clang-format`).

## Architectural principles

1. **Essentials has no dependency on WhenItFails.** The dependency direction is `when_it_fails -> essentials`, never the reverse.
2. Preserve shared **behavioral contracts** with Toolbox where appropriate; do not mechanically translate .NET code into C.
3. Every public function documents input validity, return values, errors, data lifetimes, ownership, and cleanup.
4. No external runtime dependencies in the Essentials core unless there is a compelling reason.
5. Work test-first where possible: small verified features, then real consumer integration in MemView.
6. Cross-language specifications cover observable behavior and edge cases, **not** identical in-memory layouts or ABIs.
7. Keep module boundaries clear; higher-level modules may eventually use optional external libraries without burdening the core.

Read [Architecture](Docs/ARCHITECTURE.md), [API documentation rules](Docs/API_DOCUMENTATION.md), and [Roadmap](Docs/ROADMAP.md).

## Related projects

- [.NET Toolbox](https://github.com/afrowaveltd/Toolbox) — conceptual and behavioral reference, especially `Essentials/Results`.
- [MemView](https://github.com/afrowaveltd/MemView) — educational C consumer and integration test bed.

## License

MIT; see [LICENSE](LICENSE).
