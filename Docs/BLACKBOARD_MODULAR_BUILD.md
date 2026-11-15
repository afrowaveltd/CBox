# CBox Blackboard / CBoard — modular builds and future configurator

**Status:** proposal only; the present repository Makefile is a C17 foundation/smoke-test build, **not** a working modular Blackboard build.

## Design rule

One useful CConsole **core**, many independent optional modules. A developer creating a small TUI should not be required to install a Markdown parser, a JSON/AJIS viewer, a C formatter, a code highlighter or a filesystem browser.

Prefer a **single maintained Makefile with a selected module configuration**, rather than separately maintained full Makefiles for every possible combination (combinatorial maintenance overhead). A future small TUI/CLI configurator with checkboxes can generate the configuration, validate dependencies, then invoke make. It should never generate arbitrary executable recipes from user-supplied text.

## Sketch of module graph

```text
cconsole-core (terminal IO, capabilities, styles, text, layout)
    |
    +-- cconsole-widgets (tables, panels, buttons, text, status, progress)
    |
    +-- cconsole-input (keyboard/mouse/focus/prompt)
    |
    +-- cconsole-codeblock (literal source display)
    |      +-- cconsole-format-c       [optional C source formatter]
    |      +-- cconsole-highlight-c    [optional C syntax highlighting]
    |      +-- cconsole-highlight-json [optional JSON syntax highlighting]
    |
    +-- cconsole-markdown (optional; uses CodeBlock where requested)
    +-- cconsole-structured-json (optional)
    +-- cconsole-ajis-view (optional; explicit external/native AJIS parser)
    +-- cconsole-filebrowser (optional; input/widgets + optional preview adapters)
```

Exact modules and dependencies are **not frozen**; a core-only build must remain meaningful and small. External parsers and test frameworks must not accidentally become runtime dependencies of the core. Dependency graph is directed, validated, without cycles; enabling a feature either enables prerequisites explicitly or explains missing ones. Disabling a feature must remove its runtime/link dependency wherever feasible.

## Make configuration (illustrative, not present in repository today)

```make
# build/config.mk — a tool-generated selection, not a separate Makefile
CCONSOLE_CORE        = 1
CCONSOLE_WIDGETS     = 1
CCONSOLE_INPUT       = 1
CCONSOLE_CODEBLOCK   = 1
CCONSOLE_FORMAT_C    = 0
CCONSOLE_HIGHLIGHT_C = 1
CCONSOLE_MARKDOWN    = 0
CCONSOLE_JSON        = 0
CCONSOLE_AJIS        = 0
CCONSOLE_FILEBROWSER = 0
```

```bash
# Future intended use, NOT implemented:
make CONFIG=build/config.mk
make PROFILE=minimal
make PROFILE=full
```

A stable root Makefile can include a validated `config.mk` and conditional module source/archive lists. Alternatively a configurator could generate an **included make fragment** and a `cbox_config.h` with feature flags. Keep output artifacts per profile/config hash to avoid stale object reuse between differing flags and platforms. Feature-specific static archives (`.a` / `.lib`) may be a better packaging boundary than one giant library; ensure correct transitive link flags, include paths and ABI/feature metadata.

### Suggested profile presets

- `minimal`: core markup, theme, plain and terminal writers only.
- `tui`: core + widgets + interaction/focus.
  - A `Button` is a widgets module renderable; **activation** requires the input/focus module, while drawing a button does not. No new runtime dependency on a mouse or ncurses.
- `docs`: tui + CodeBlock + Markdown, optional highlighters.
- `full`: all modules whose declared dependencies are satisfied, not automatic installation of arbitrary external parsers.
- Custom checkbox selection: validated per-feature configuration.

A profile describes a **build**, not a runtime feature toggle. Unsupported features should be absent or report unsupported cleanly, not link accidentally and fail much later.

## Configurator app (later)

- Small interactive terminal app, eventually **built with CConsole itself**, showing checkboxes for modules and relevant flags.
- CLI bootstrap must exist first so CConsole can build without needing an already-built CConsole configurator: avoid a circular bootstrap dependency.
- Display module descriptions, selected size/dependency information **only if measured/known**, and reasons a feature is disabled.
- Generate configuration, show/confirm resolved dependency closure, then optionally invoke the existing build target. Keep logs, exit status, diagnostics and output paths visible.
- No invoking a shell with unchecked user-controlled commands or paths; safe argument handling and predictable output location.
- The default build and test flow must still work without this app, on Debian and Windows toolchains.

## Resource and quality policy

- Choose dependency footprint from required features; don't drop genuinely useful behavior to shave a few KB.
- Static modules may allow linkers to remove unused sections with appropriate build flags, but **don't rely** on that as the only module boundary.
- Every supported preset needs smoke/unit/integration tests; test representative combinations and deliberately incompatible configurations.
- Verify no unwanted dependencies, missing symbols, duplicate symbol collisions, differing compiler/standard flags or stale cached outputs.
- Publish reproducible build options and artifact manifest (selected modules, compiler, target, C17 flags, versions).
- Introduce install/package naming only after public APIs and module contracts stabilize.

Related: [CodeBlock formatting/highlighting](BLACKBOARD_CODE_BLOCKS.md), [Blackboard design](BLACKBOARD_CONSOLE_DESIGN.md), [CBox roadmap](ROADMAP.md).
