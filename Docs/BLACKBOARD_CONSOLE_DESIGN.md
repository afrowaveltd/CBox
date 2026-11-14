# CBox Blackboard — Spectre.Console-compatible terminal UI vision

**Status:** Architecture and implementation roadmap only. There is NO Blackboard C module, stable public API, widget implementation or verified C UI parity yet.

**Objective (14 November 2026):** Build a reusable native **C17** terminal UI library whose behavior, markup syntax, widget vocabulary, themes and visual results match **Spectre.Console** as closely as technically practical — not merely a lightweight color formatter. Users should be able to build aesthetically consistent installers, test runners, CLI tools and dashboards in C and C#.

See [the detailed compatibility tracker](BLACKBOARD_SPECTRE_PARITY.md). A previous tiny-writer-only goal is superseded by this document; a simple writer is still the *first implementation slice*, never the *project's feature ceiling*.

## Reference project and compatibility policy

- Reference implementation: https://github.com/spectreconsole/spectre.console
- Reference documentation: https://spectreconsole.net/console/
- Markup grammar: https://spectreconsole.net/console/reference/markup-reference/
- Rendering model: `src/Spectre.Console/Rendering/IRenderable.cs` and renderable widgets (e.g. `Widgets/Panel.cs`).
- ANSI/markup internals: `src/Spectre.Console.Ansi/AnsiMarkup.cs`, `AnsiMarkupTagParser.cs`, `AnsiWriter.cs`, `AnsiDetector.cs`.
- Interactive features: selection, text and multi-selection prompts; progress/status/live interfaces.
- Spectre.Console documentation currently labels the project as **v0.57**; use that as a *comparison snapshot*, not a claim that all future versions will stay identical.

**Target visual/behavioral compatibility**, not source-code identity, identical API syntax, binary ABI or perfect glyph matching across arbitrary fonts and terminal emulators. C has no C# generics, fluent extension methods or automatic object lifetimes; design an idiomatic C API that produces comparable results. Never claim a feature compatible without tests.

Respect upstream copyright/license terms when adapting any actual source; prefer independently written C17 implementations informed by behavior and tests, with appropriate notices if code is reused.

## Dependency architecture

```text
C applications: MemView, installer, monitor, test runner
        |
        v
 CBox Blackboard public facade (C17)
        |
        +-- markup parser / style / color / themes
        +-- renderable model / segment representation
        +-- text width / wrap / clipping / layout
        +-- widgets: tables, panels, trees, charts, ...
        +-- live/progress/status and prompts
        +-- screen diff / cursor / input orchestration
        |
        +-- terminal backends:
             plain output | ANSI / VT | POSIX adapter | Windows adapter
```

Blackboard must remain independent of CBox `essentials`, `when_it_fails` and the **external** DoesItFail testing framework. The core renderer uses C17 and a simple abstract output interface; some operating-system integration necessarily uses *small separate* POSIX or Windows translation units. **No mandatory ncurses dependency**. Optional integrations can build on Blackboard, not be required to use it.

Blackboard's own test executables MAY link DoesItFail; the production library MUST NOT. An optional DoesItFail console reporter may in the future link Blackboard while the DoesItFail core stays independent.

## Architectural layers (do this before advanced widgets)

### 1. Terminal capabilities and output context

Explicit `cbox_blackboard_console` instance with borrowed stream or caller-defined writer, terminal dimensions, ANSI enabled, color depth, theme, cursor state and output policy.

- Automatic/basic/256/truecolor support with overrides; respect `NO_COLOR`, redirected output, `TERM=dumb`, and user preferences.
- A redirected pipe or log receives plain-readable text, not raw control codes by default.
- `clear_screen`, cursor movement, hide/show and optional alternate screen are **capability-checked operations**, not unconditional escape sequence writes.
- Windows Console VT mode via platform adapter when supported.
- The console's background color is generally **not reliably discoverable**. Set readable foreground/background pairs in optional themes; `ESC[0m` restores terminal defaults, not necessarily black on white.
- Explicit enter/leave UI session restores cursor visibility and terminal modes on normal cleanup; document best-effort limitations on hard crashes.

### 2. Markup and styled segments

- Parse input incrementally (text spans, opening/closing tags, escapes, named colors, hex RGB, background `on`, styles like `bold`/`underline`/`dim`).
- A **style stack** restores the preceding composite style when `[/]` closes nested tags.
- Convert markup into styled segments (text slice + style + metadata). Do not mix parsing with direct terminal output.
- Escape untrusted content; specify malformed-tag behavior. Do not assume `{count}` can access a C variable by name: any interpolation layer needs explicitly supplied typed values.
- Allow streaming output and bounded parsing with predictable allocations. Data bytes and control-code bytes must have clear policies.

### 3. Renderables and measurement

Adopt Spectre.Console's key *concept*, not its .NET interface shape: `measure(max_width, capabilities)` plus `render(available_width, ...)` on a C renderable descriptor / vtable or tagged union; decide after first prototype and tests.

- A styled line/segment representation is a shared intermediate format for text, markup, widgets and test snapshots.
- Layout primitives: wrapping, padding, alignment, borders, columns, grids, vertical and horizontal layouts, clipping and width/height limits.
- Correct terminal **cell width** is distinct from UTF-8 byte length, Unicode codepoint count and grapheme-cluster count. Combining marks, double-width CJK, emoji, and ANSI escapes demand deliberate tests/fallbacks.
- Consider a bounded frame buffer or line-based render output for screen refresh, not a full unbounded copy for every `write`.
- Build a plain-text backend and a **recording backend** first-class so rendering can be tested without a physical terminal.

### 4. Widgets

Plan toward the real Spectre widget family: markup/text, rule, panel, table, grid, columns, rows, alignment/padding, tree, progress bars, calendars, charts, visual layout, custom renderables and dashboard composition. A compatibility tracker must identify variants, styling options and tests instead of counting superficial function names.

### 5. Interaction and live display

- Prompts: confirmation, text input/validation, selection and multiselection, with key event abstraction.
- Progress/status/spinners: multiple tasks, update rates, cancellation policy and rendering when output is redirected.
- Live screen: exclusive ownership of dynamic drawing to avoid clashing with prompts and other live displays; delta redraw is a later optimization.
- Inputs use raw/cooked terminal mode in controlled adapter code. All mode modifications must have well-defined restoration and failures.
- Multiplexed printing/logging during live rendering needs an explicit policy (queueing or locking, never accidental unsynchronized escape bytes).

## Usability and idiomatic C API (not yet implemented)

Public exported symbols follow the CBox-required `cbox_` prefix. Choose stable names based on the familiar Spectre.Console vocabulary (Markup, Text, Panel, Table, Tree, Prompt, Status, Progress, Layout, Live) and keep common user spellings intuitive.

**Illustrative signatures — DO NOT copy into consuming code as though implemented:**

```c
typedef struct cbox_blackboard_console cbox_blackboard_console;
typedef struct cbox_blackboard_renderable cbox_blackboard_renderable;

int cbox_blackboard_markup_line(cbox_blackboard_console *console,
                                const char *markup);

int cbox_blackboard_render(cbox_blackboard_console *console,
                           const cbox_blackboard_renderable *widget);

int cbox_blackboard_clear_screen(cbox_blackboard_console *console);
```

Object ownership, status codes, null handling, builder functions, `FILE *` borrowing, escaped versus markup text and cleanup must be fixed **before** the public API is frozen. A convenience facade can be provided over a context-based API, but avoid hidden mutable global context for multi-console support.

Suggested user experience (conceptual):

```c
/* Corresponding *intent* to Spectre.Console examples, not final C API: */
cbox_blackboard_markup_line(console, "[bold blue]Starting[/] [green]OK[/]");

/* A table widget would be configured, rendered, then destroyed explicitly. */
cbox_blackboard_render(console, table);
```

The user's proposed `console_write` / `console_writeline` can remain simple educational aliases; a public C library must avoid collisions with unrelated symbols. Prefer `clear_screen` rather than `clean_screen` for clearing the terminal.

## Quality constraints

- Baseline C17, strict compiler warnings, predictable ownership, Allman braces, four-space indentation, English documentation.
- Work on Debian/Linux + Windows console/terminal; validate on real terminal emulators rather than trusting one VT implementation.
- Useful on low-resource devices: stream plain text, allocate when rendering complex widgets, bounded caches and adjustable refresh rates.
- Support 80×24 and smaller terminals, handling resize and narrow widths without hangs or out-of-bounds writes.
- The same **defined theme** and a consistent feature set should give very similar output in C and C#; glyph appearance and font metrics can vary by terminal.
- No runtime dependency on Spectre.Console or .NET; no mandatory ncurses.
- Test invalid markup, excess nesting, escape injection, Unicode widths, large content, file redirection, terminal errors, background contrast, resize, cleanup and allocation failures.
- Document licensing; maintain clear notices for any adapted code.

## Development strategy: full target, vertical slices

We will build the *foundation for the complete system* from day one, but implement it in small steps the learner can understand. A rich console engine is not achieved by writing every widget independently.

**Phase A — Reference and contracts.** Inventory Spectre.Console capabilities; define screenshot/ANSI/semantic compatibility expectations and ownership rules. Implement benchmark/reference fixtures in C# and expected C results.

**Phase B — ANSI, markup, output contexts.** First learner exercise: streaming parser with a few tags. Expand to style stack, backgrounds, literal escapes, RGB and detection. This is a subset **of a complete architecture**, not a limited final product.

**Phase C — Common renderer.** Styled segments, text measurement, UTF-8 width policy, layout primitives, headless recording backend. Start golden tests against C# and plain mode.

**Phase D — Static rich widgets.** Text/rule, panel, table, tree, grid/columns/rows, layout; snapshot appearance compared with Spectre.

**Phase E — Dynamic widgets.** Progress/status/spinners, live display and safe partial screen refresh; ensure log and stream redirection behavior.

**Phase F — Interactive widgets.** Input events, prompts, selection/multiselection, validation, keyboard handling, safe terminal-mode transitions.

**Phase G — Remaining parity/packaging.** Charts/calendar, special rendering, alternative terminal features, benchmarking, cross-platform tests and install/build packaging.

Each phase must update [the compatibility matrix](BLACKBOARD_SPECTRE_PARITY.md). A widget is **implemented** only after tests and a real consumer scenario pass. Declare nonmatching behavior and unsupported features explicitly.

## High-priority advanced component: FileBrowser

The existing nearly finished **C# Spectre.Console FileBrowser** is an important **product-specific reference**, alongside upstream Spectre.Console itself. Our goal is to reproduce its interface and practical behavior in a native C17 counterpart, not merely create a directory-listing demonstration.

**Architecture rule:** FileBrowser is a higher-level module built on the CConsole/Blackboard UI engine, not a runtime dependency of the core renderer. File-system access is performed through a separately testable platform abstraction. The ANSI/renderable core must remain useful without file-system functions.

Proposed module structure (paths and symbols not yet implemented):

```text
blackboard/
  include/cbox/blackboard/console.h
  include/cbox/blackboard/file_browser.h
  src/core/                   # terminal, styles, layout, input
  src/widgets/                # reusable table, panel, tree, prompt, ...
  src/file_browser/           # browser navigation, selection, views
  tests/file_browser/
```

Recommended **feature targets**, to verify against the actual C# repository before claiming parity:

- Browse directories, highlight current entries, keyboard navigation, choose files/directories, cancel without changing caller state.
- Sort/filter/search, visible file metadata and clear errors for inaccessible paths or I/O failures.
- Preview text/Markdown/code when matching C# reference capabilities are established. Large files should use bounded reading, paging and/or optional disk cache; no requirement to load an entire large file into RAM.
- Responsive terminal layout, resize handling, keyboard hints, Unicode and readable theme.
- Platform-specific path semantics (POSIX, Windows), symlinks/junctions, permissions and safe navigation.
- Optional future URL/Git remote preview only where backed by an explicit provider, not a mandatory dependency of the local file picker.
- Borrowed/output ownership, result selection and cancellation contracts must be explicit and tested.
- Avoid mandatory ncurses or network dependencies; previews and other adapters should be optional components.

**Dependency direction:** `file_browser -> Blackboard renderables/layout/input -> backend`; never `renderer -> file_browser`. FileBrowser can be packaged separately while remaining part of the Blackboard family.

**Reference verification status:** The presumed repository `afrowaveltd/SpectreConsoleFileBrowser` could not be opened with the current GitHub connection during this review. Do **not** record individual C# behaviors as verified from that repository until we can access its exact source. Port from the user's current real implementation, preserving behavior and tests, once the reference repository is accessible.

**Phasing:** Implement after basic renderables, layout, keyboard input and terminal resize are stable. A first file-picker vertical slice may be used to validate those abstractions **before** all advanced Spectre.Console widgets are complete. Test both interactive and headless file-system logic.

## High-priority optional viewer: AJIS and JSON

The CBox Blackboard scope also includes a **common AJIS/JSON structured-data viewer**, fed by an external native AJIS parser when its stable C API becomes available. This is intended to surpass simple colorized JSON with interactive tree navigation and low-memory file previews.

**Do not merge parsing into rendering:** `AJIS parser -> independent data API`; the optional `Blackboard AJIS viewer -> AJIS parser + Blackboard core`. FileBrowser is one consumer of the viewer, not its owner. Keep a strict JSON mode distinct from AJIS-specific extensions.

For interfaces, memory constraints, lazy/streaming design, testing and source/view split see [AJIS + JSON Viewer](BLACKBOARD_AJIS_VIEWER.md). The native parser and viewer are both **planned**, not already implemented.

## Real consumers and shared appearance

1. **Blackboard's own demonstration app** as a visual comparison with an equivalent Spectre.Console C# demonstration.
2. **DoesItFail console reporter (optional)** while the DoesItFail core and its original stdio reporter remain independent.
3. CBox command-line tools and instructional samples.
4. **FileBrowser**: reproduce the current C# Spectre.Console browser in C17, reusing widgets and input adapters.
5. **AJIS/JSON Viewer**: one structured-data explorer shared by standalone CLI tools and FileBrowser preview, once the parser API is available.
6. MemView, diagnostic dashboards and installers that need panels, prompts, and live updates.

Make a shared theme specification (named colors/hex values, border characters, spacing, fallback glyphs), **not** a promise that two terminals will render identical pixels. Shared golden examples should compare stable styled segments, normalized ANSI output or final cell grids and measure resulting readable layout.

## Learning principle

The learner writes useful small increments (first parser, style stack, simple renderable); explain the C mechanism (pointer ownership, callbacks, struct layouts and error handling), define tests first, and integrate only after confirming actual build results. The ultimate parity objective does not imply skipping learning steps.

## References reviewed

- https://spectreconsole.net/console/
- https://spectreconsole.net/console/reference/api/
- https://spectreconsole.net/console/reference/markup-reference/
- https://spectreconsole.net/console/widgets/panel/
- https://spectreconsole.net/console/widgets/layout/
- https://spectreconsole.net/console/live/live-display/
- https://spectreconsole.net/console/how-to/prompting-for-user-input/
- https://github.com/spectreconsole/spectre.console
