# CBox roadmap

This is a proposed **learning and implementation order**, not a release promise.

## Phase 0 — Repository foundation

- [x] Repository and MIT license created
- [x] Module boundaries and dependency direction documented
- [x] Folder skeleton and placeholders committed
- [x] Basic `.gitignore`, `.editorconfig`, and Allman `.clang-format`
- [x] C17 smoke-test target and initial documentation
- [x] Confirm fresh Debian 13 clone and `make help` / `make test` (2026-11-13; GCC/cc C17 smoke test passed on SLAVEK-LIN)

## Phase 1 — Essentials: Result

- [ ] Compare Toolbox `Result` behavior with desired small C API
- [ ] Decide status and error-description representation
- [ ] Document ownership/lifetime and invalid-input contracts
- [ ] Write unit tests and first language-neutral test scenarios
- [ ] Implement Result in C17 without external runtime dependencies
- [ ] Use Result in a real MemView operation

## Phase 2 — Essentials: Response

- [ ] Decide how typed payloads are represented in C
- [ ] Establish empty/success/failure and payload ownership semantics
- [ ] Test payload lifetime, invalid states, copying, and cleanup
- [ ] Implement minimal Response and usage examples
- [ ] Use Response in MemView

## Phase 3 — WhenItFails

- [ ] Define responsibilities that do not belong in Essentials
- [ ] Reuse Essentials rather than introducing reverse dependencies
- [ ] Align error scenarios with Toolbox where appropriate
- [ ] Test independent builds and MemView integration

## Phase 4 — Cross-language interoperability at the behavioral level

- [ ] Shared specification schema and versioning
- [ ] Reusable fixtures for CBox and Toolbox
- [ ] Consider AJIS conformance tests where meaningful
- [ ] Add other language implementations only when maintainers want them

**Non-goals for foundation:** port every .NET module, promise a stable ABI, or create a heavyweight framework before one useful type has been proven.

## Parallel development tool — DoesItFail (independent repository)

- [x] Choose independent library architecture and development-only dependency direction
- [x] Prepare opt-in CBox static-library integration and version-controlled VS Code tasks
- [x] Document integration, C17 portability policy, and AJIS/AjisFS exploratory notes
- [x] Publish independent DoesItFail repository at https://github.com/afrowaveltd/DoesItFail (C17 educational seed; no stable release)
- [ ] Verify complete first Result implementation and commit learner-owned `result.c`
- [ ] Gradually migrate Result tests from plain `assert` to DoesItFail EXPECT checks
- [ ] Add more assertion types and optional ncurses reporter **in DoesItFail**, not CBox
- [ ] Introduce packaging/install rules once the public API is stable

## Parallel module proposal — Blackboard: Spectre.Console-grade TUI for C

**Objective upgraded 2026-11-14:** broad behavioral and visual Spectre.Console parity, not just a small ANSI color writer. See [architecture](BLACKBOARD_CONSOLE_DESIGN.md) and [feature parity tracker](BLACKBOARD_SPECTRE_PARITY.md).

- [x] Inspect CBox module boundaries and actual Spectre.Console markup/rendering/widget designs.
- [x] Define independent C17 core plus thin POSIX/Windows terminal adapters; no mandatory ncurses, Essentials or DoesItFail runtime dependencies.
- [x] Establish long-range feature matrix for markup, tables, panels, trees, charts, live displays, progress, prompts and layouts.
- [ ] Freeze documented API/ownership and error handling contracts for initial output context and parser, plus C# reference test fixtures.
- [x] Document built-in dark theme, optional INI configuration, interactive-only clear-on-init, shutdown and semantic markup roles in [theme design](BLACKBOARD_THEMES_AND_INIT.md).
- [x] Specify the proposed optional [AS/400 showcase and theme](BLACKBOARD_AS400_SHOWCASE.md), including focusable table and footer.
- [x] Specify [printf-style and explicitly bound typed named placeholder](BLACKBOARD_FORMATTING.md) design, including literal-data safety and the limits of C reflection.
- [x] Refine placeholder UX: positional `{label}` consumption left-to-right with a C17 `_Generic` macro facade; no required `CBOX_INT(...)` wrapper in common use.
- [ ] Prototype typed macro argument mapping (bounded arity), single evaluation, mismatch diagnostics and passing literal user strings without markup injection.
- [ ] Implement/test compatible `%d`/`%s` formatting and named `{key}` binding without treating substituted strings as markup; finalize APIs after proof-of-concept.
- [ ] Implement and test AS400 demo as a consumer of real table/theme/input components; validate classic 80x24, narrow, monochrome and redirected modes.
- [ ] Test theme configuration/override precedence, whole-screen background fill on capable/legacy terminals, nested role resolution, automatic plain-text fallback and terminal restore semantics.
- [ ] Learner implements first ANSI/markup writer slice (`[red]`, `[blue]`, `[/]`), with tests and no hidden state.
- [ ] Build renderable/measurement/layout/Unicode cell-width foundation early enough to support all widgets.
- [ ] Implement and benchmark **render-and-forget** streaming output (no retained write history, minimal allocation and ANSI transitions) separately from optional viewport-sized live redraw buffers.
- [ ] Establish performance baselines: allocations, peak RSS, ANSI bytes, write/flush counts, frame time and resize behavior, with identical scenarios for Spectre.Console and CConsole.
- [ ] Add static widgets gradually (text, rule, panel, table, tree, grid, layouts) with normalized comparison tests against Spectre.Console.
- [x] Document [CodeBlock formatting vs optional syntax-highlighter module](BLACKBOARD_CODE_BLOCKS.md), preserving literal source and applying styles via offset spans, not embedded markup tags.
- [x] Clarify that an **optional language-aware source formatter** may transform a temporary working copy (indent/brace/line style), while the highlighter applies spans over **the displayed copy**; original source stays immutable.
- [x] Record proposed [modular build profiles and future checkbox configurator](BLACKBOARD_MODULAR_BUILD.md) with a single root Makefile and validated dependencies.
- [ ] Implement a minimal core-only build, then independent optional module archives/configuration; test absence of unselected module dependencies.
- [ ] After CConsole input/widgets stabilize, prototype a configurator TUI generating a validated make config and invoking the existing build, with a noninteractive CLI path.
- [ ] Implement literal, preformatted CodeBlock first (spaces, tabs, blank lines, horizontal clipping), then optional style-span highlighters with source-preservation tests.
- [ ] Add progress, status, live rendering and screen lifecycle with redirect/fallback safeguards.
- [ ] Add prompts, input, keyboard navigation and validation with reliable terminal-mode restoration.
- [ ] Design and test clickable **action regions**: keyboard activation, mouse reporting (SGR 1006/Win32), UTF-8-safe hit testing, callback context lifetimes, input mode restoration and safe OSC 8 URL distinction.
- [ ] Add charts, calendar, more styles/themes, packaging and cross-platform examples.
- [ ] **High priority: CConsole FileBrowser**, port the nearly finished Spectre.Console FileBrowser using shared rendering/input/layout; verify the exact C# reference repository and its existing features first.
- [ ] Cover file navigation, keyboard selection, cancellation, previews where supported, long-file/paging, terminal resize and filesystem error cases without adding dependencies to rendering core.
- [ ] **AJIS/JSON Viewer:** define parser-independent token/navigation adapter and strict JSON vs AJIS dialects; see [viewer design](BLACKBOARD_AJIS_VIEWER.md).
- [ ] Add styled JSON output, interactive AJIS/JSON tree navigation, sparse/streaming handling for very large files and optional FileBrowser preview integration.
- [ ] Integrate the actual AJIS parser once its C API and behavior are verified; do not put parser code into Blackboard core.
- [ ] Use in CBox programs, MemView and optionally a DoesItFail **reporter**, while DoesItFail's core stays standalone.
