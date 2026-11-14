# DoesItFail: external testing library integration

**Status: optional test-only integration.** CBox's Result teaching exercise is deliberately unchanged. The learner's local `essentials/src/result.c` is not overwritten.

## Responsibilities

- **DoesItFail** is a separate, reusable C17 library for test execution, assertions, structured failure events, and multiple reporters. It is **not part of CBox**.
- **CBox/testing** means CBox-owned test cases that consume DoesItFail; those tests belong to CBox, not the framework.
- **CBox/when_it_fails** handles application failures, not unit-test execution.
- **CBox/essentials** must never depend on DoesItFail in production builds.
- **.NET Toolbox** keeps its normal .NET test tooling. Share behavioral cases when useful, not binary layouts or build tools.
- Console output and optional future ncurses UI are separate reporters; the test execution core does not own the terminal.

## Local layout and commands

```text
Afrowave Projects/
    CBox/
    DoesItFail/
```

```bash
# From CBox's parent directory, if not already cloned:
git clone https://github.com/afrowaveltd/DoesItFail.git
cd DoesItFail
make all
make test

cd ../CBox
make test             # no external dependency
make test-framework   # compiles DoesItFail as a sibling and verifies linking
make test-result      # existing learner-owned Result code; needs local result.c
```

If projects are not siblings:

```bash
make test-framework DOES_IT_FAIL_DIR='/absolute/path/to/DoesItFail'
```

`make test-framework` is deliberately optional and returns nonzero for missing dependency or test failures. It tests integration only, **not Result functionality**. No `ncurses` is linked. The upstream project is now published at [afrowaveltd/DoesItFail](https://github.com/afrowaveltd/DoesItFail); this seed is **not a stable API release**.

## VS Code: Debian and WSL

The checked-in `.vscode/settings.json`, `c_cpp_properties.json`, `tasks.json`, and `launch.json` configure Microsoft C/C++ IntelliSense, GCC C17, a GDB launch profile, and repeatable make tasks. They include `../DoesItFail/include` for a sibling checkout.

Open the **CBox directory itself** as a workspace. Use *Terminal → Run Task* for:
- CBox: smoke test
- CBox: Result tests
- CBox: DoesItFail integration
- CBox: clean

Use *Run and Debug* after running/compiling the chosen test. The Result debugger depends on a locally implemented `result.c`. For Windows-native compilers change `compilerPath`, `intelliSenseMode`, `miDebuggerPath`, and the Make program as needed; these tracked defaults are designed for Debian/WSL.

The editor-specific local state remains ignored; only the four shareable configuration files are version-controlled.

## Repeatable procedure for every new CBox module

1. Record its purpose, target standard, memory ownership and production dependencies.
2. Add public headers and production code with C17 / Allman / `cbox_` conventions.
3. Add module-owned tests; reuse DoesItFail, do **not** duplicate its runner.
4. Update Makefile targets, VS Code tasks/debug profile, README, architecture/API docs, and roadmap **in the same change**.
5. Run `make test`, relevant module tests, and optionally `make test-framework`; confirm exit statuses.
6. Keep the educational method: theory → understanding → learner implementation → tests → real integration.
7. Promote optional C99 compatibility only after actual compilation/testing on the relevant toolchain.

Current DoesItFail seed: typed integer equality (`DIF_EXPECT_INT_EQ`), callback-based runner, console reporter with opt-in ANSI color, and independent bootstrap tests. Typed string/memory checks, `REQUIRE`, a native ncurses reporter, releases, and package managers remain future work.
