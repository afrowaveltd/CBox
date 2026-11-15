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

## Performance and memory policy: render and forget wherever possible

**Design goal:** exceptionally low baseline memory, no unnecessary retained output, low and predictable allocation counts and efficient terminal I/O, **without claiming unmeasured superiority over .NET or ncurses**. C17 makes resource control explicit; actual performance depends on algorithms, Unicode handling and terminal/SSH throughput.

### Mode A — immediate / streaming output (default for ordinary writes)

- Parse borrowed input text incrementally using a cursor, with a small current-style state and a bounded nested-style stack.
- Emit contiguous source spans with `fwrite` (or a caller-supplied buffered writer) and only essential ANSI style transitions; avoid `printf`/syscall per character.
- Do not allocate a copy of the input, create a complete syntax tree or retain printed content after the write finishes.
- Avoid heap allocations for the common case when possible; the output context can borrow an existing `FILE *`. Define safe behavior for excess nesting rather than silently overflowing a fixed stack.
- **Fire-and-forget means no retained rendered data; it does NOT mean ignore I/O errors.** Return/document write failures and preserve best-effort terminal style restoration.

### Mode B — transient widget rendering

- A static table/panel may measure and emit lines progressively without storing a second full-screen buffer, provided cell widths, wrapping and alignment have been determined.
- Select minimal temporary scratch storage for columns/rows and release it as soon as the render call completes.
- Input widgets and data sources may have their own memory ownership; do not assume table data magically disappears.

### Mode C — interactive/live rendering

- Keep **only essential application/view state**: focused action identity, scroll offset, visible data/window and necessary object handles. Reconstruct the screen as needed.
- For efficient redraw, optionally keep a **viewport-sized previous-frame buffer**, not a history of every screen or an entire large document. Dirty regions and/or cursor-local updates reduce unnecessary writes.
- On a terminal resize or theme change, recompute layout from the retained model/data source; never rely on the terminal to reconstruct previously discarded output.
- Lazy source paging/sparse indexing for large FileBrowser directories and AJIS documents must be decided per data source. Enumerating every directory item repeatedly can be slower than a bounded index; *no-cache everywhere* is not an optimization.
- Maintain hit regions for currently visible action targets. Rebuild or invalidate after layout/scroll changes; do not retain stale callback/context pointers.
- Provide explicit memory budgets and optional bounded caches. Retained caches must be measured against improved redraw latency and I/O cost.

### I/O and benchmark contracts

- Minimize redundant ANSI sequences, large flush counts and full-screen rewrites. Terminal I/O and layout, not CPU alone, can dominate end-to-end performance.
- Measure **elapsed time, bytes emitted, number of writes/flushes, allocations per operation, peak resident memory and frame latency** separately. Compare equivalent behavior, text and terminal settings with Spectre.Console and with an ncurses example before drawing conclusions.
- Test small and constrained terminals (80x24, 40 columns), low-memory devices and redirected output. Include long Unicode strings, multi-codepoint emoji, nested styles, millions of lines and error/abort paths.
- Keep debug/recording/snapshot backends possible without forcing production output to retain full rendered history.

**Rule:** prefer output streaming by default; retain data only when needed for correct interaction, efficient repaint, reflow after resize or explicit user-visible features. Allocate by *actual visible complexity* wherever feasible rather than total file or output history.

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

## Interactive action regions and mouse support (design candidate)

**Goal:** in addition to plain OSC 8 URL hyperlinks, support **actionable text, menus, links and widgets backed by user-provided C function pointers**. This belongs in the core event/render architecture, not as a renderer-specific shortcut.

**Distinguish two separate protocols:**
- **OSC 8 hyperlink** attaches a URI to terminal output. The **terminal** normally owns link activation; clicking it does **not** send a C function pointer or a callback invocation to the app. Spectre.Console's `AnsiWriter` already has `BeginLink`/`EndLink` for this protocol.
- **Terminal mouse reporting** (xterm-style `DECSET 1000` with SGR extended mouse encoding `1006`) sends mouse button/position reports back **to the application** where supported. A backend parses the report into a neutral input event. Windows native console input can also provide `MOUSE_EVENT_RECORD` via `ReadConsoleInput`. Feature detection and mode enablement are platform/backend-specific; support is not universal.

**Core architecture addition:**

```text
terminal input (VT sequence / Win32 input event)
    -> input decoder (key, mouse, wheel, resize, focus, paste)
    -> normalized event queue
    -> screen coordinate hit-testing against action regions
    -> focus/navigation dispatcher
    -> action callback + context pointer
    -> state update / render invalidation / redraw
```

The renderer must associate its final *terminal cell bounds* with an action identity (including wrapped or multi-line text, broad emoji and clipped widgets). Hit testing must be derived from **actual post-layout positions**, never source-string character offsets; otherwise Unicode and scrolling would break clicking.

**Illustrative callback concept — not public API:**

```c
typedef void (*cbox_console_action_fn)(void *user_data);

typedef struct
{
    const char *label;                 /* Borrowed while registered */
    cbox_console_action_fn callback;  /* Non-NULL while active */
    void *user_data;                   /* Borrowed context */
} cbox_console_action;
```

The caller retains ownership of `user_data` unless a future API explicitly transfers ownership. Region/handler unregistration must be defined so the framework cannot invoke a callback after its context or view has been destroyed. Reentrant callbacks, updates during dispatch and concurrent rendering need an explicit policy. Do not invent opaque pointer values as escape-sequence payloads.

**Interaction principle (agreed focus/activation model):** active regions are first-class **focusable UI elements**. The same application-defined action handler is invoked by clicking an active area and by using the keyboard. The intended default navigation is **Tab** = next focusable item, **Shift+Tab** = previous item, **Space** = activate focused item. **Enter** may also activate controls where appropriate. A terminal with no mouse can still operate every critical workflow. Noninteractive/plain output shows the label without pretending it is an interactive hyperlink.

- **Underline is presentation, not behavior:** an underlined text span may optionally be registered as an active action target, but underlined text is not automatically clickable. Action regions may also cover table rows, tree nodes, buttons, menus and other renderables.
- **Focus state is explicit:** preserve deterministic focus order, skip disabled or hidden entries, visibly mark focused element, keep it visible when scrolling, and restore focus appropriately after UI changes.
- **Focus styling with cell backgrounds:** a focused action may use a configurable foreground/background pair, underline, border and/or visible focus marker. ANSI SGR standard backgrounds are 40–47/100–107, indexed colors via 48;5;n and RGB via 48;2;r;g;b; SGR 7 reverse-video is another fallback. The theme must preserve readable text/background contrast (avoid assuming the user's terminal background). Use theme and terminal capability to choose the style.
- **Whole-row selection:** to highlight a complete table/menu row, the renderer must paint all of its display cells, including trailing spaces to the row's layout width. Setting a background style for the label alone does not automatically fill the unused rest of the terminal line. Wide Unicode glyphs occupy multiple cells and should retain the same action/focus style throughout their cell span.
- **Legacy/plain fallback:** when ANSI backgrounds are absent, indicate focus visibly with a text marker such as `>` or `[selected]`; do not make color the only focus signal. Keep mouse and keyboard selection state separate from hover/focus presentation.
- **Terminal style state:** SGR 49 restores default background only; SGR 0 resets all attributes. For nested markup and widgets restore the previous composed style where applicable, not indiscriminately `SGR 0` in the middle of a styled parent.

- **Activation is unified:** normalized mouse click and keyboard Space/Enter dispatch an action identity to the **same** registered C callback/context. Widget authors should not implement parallel independent handlers.
- **Hit regions use actual cell coordinates after layout:** text wrapping, scrolling, terminal resize and wide Unicode characters must map to the correct active item. A click on any displayed cell of an emoji or active row should resolve consistently.
- **Bindings can be scoped to a view:** when a view is destroyed, its interactive region and callback registry entries are invalidated. Explicit borrowed-context lifetime and registration removal must be tested.
- Avoid hijacking Space in text-editing controls; the focus manager must permit widget-specific key handling. Tab navigation should be predictable, and input modes should be restored on exit.


**Security/accessibility:**
- Mouse capture may override regular terminal selection and some native hyperlink behaviors; enable it only for interactive sessions and restore prior input modes on exit, including error paths when possible.
- Never interpret a string from a file, JSON/AJIS value or untrusted markup as an instruction to execute a callback, shell command or arbitrary URL without an application-approved registry/handler policy.
- Separate external URL navigation from application callbacks and explicit user action; avoid silent process execution.
- Keep recognizable keyboard focus/highlight and an ASCII fallback for legacy consoles.
- Event APIs and handler ownership are architectural requirements **but signatures and click protocols are not frozen**.

Reference:
- https://invisible-island.net/xterm/ctlseqs/ctlseqs.pdf (mouse protocol)
- https://learn.microsoft.com/en-us/windows/console/getconsolemode (Win32 mouse and VT input modes)
- https://github.com/spectreconsole/spectre.console/blob/main/src/Spectre.Console.Ansi/AnsiWriter.cs (OSC 8 hyperlink support)

## Theme-aware initialization and semantic markup

The Blackboard/CConsole foundation will include a **built-in dark theme** (nearly black/dark gray background and readable foreground), optional theme file overrides (simple INI format proposed) and **semantic inline roles** such as `[Info]`, `[Warning]`, `[Error]`, `[Quote]`, `[Code]` and `[Success]`. These extend Spectre.Console-style explicit colors, not replace them.

A high-level `init()` convenience API should establish an **interactive** session, apply the selected/default theme and clear the screen; ordinary output or redirection should not unexpectedly clear a file/pipe or overwrite the user's terminal theme. The context-based low-level API must also support no-clear mode, caller-supplied stream, shutdown and mode/style restoration. Setting SGR background alone is not sufficient to fill a whole screen on every terminal.

Keep the theme **resolved once per initialization/reload**, not loaded for every line. The tokenizer emits markup events; an independent style resolver applies semantic roles with nested-style inheritance. `[Code]` and `[Quote]` first mean inline style only; later rich code/quote widgets are separate features.

See [Themes, semantic markup and initialization](BLACKBOARD_THEMES_AND_INIT.md) for examples, proposed INI syntax, guarantees, portability and open decisions.

## AS/400-style showcase and formatting support

A built-in **AS400-themed demonstration** will use the same generic CConsole table, layout and focus components; it is a nostalgic UI example, **not** an IBM 5250 protocol implementation. The theme is optional and does not replace the default modern dark profile. See [AS400 showcase](BLACKBOARD_AS400_SHOWCASE.md).

The markup-facing API shall support **printf-style `%d` formatting** and separate **typed named placeholders** such as `{apples_count}` with an explicitly bound value (the library cannot dereference a C local variable by its runtime name). Markup interpretation applies to the trusted format template, **not** to interpolated user data. Literal text output remains an independent operation. The implementation must not choose an unsafe `vsnprintf`-then-markup-parse shortcut that reinterprets untrusted values. See [Formatting and placeholders](BLACKBOARD_FORMATTING.md).

### Ergonomic positional named placeholders

The preferred convenience syntax is `console_writeline_named("[Info]{label}: {count}[/]", value, count)`: brace labels are **positional and left-to-right**, not runtime variable-name lookup. Capture argument types at compile time using C17 `_Generic` inside a macro front-end, constructing typed descriptors for a real renderer function. The ordinary typed bindings remain a useful lower-level API. There is no automatic dereference of arbitrary pointer types: caller writes `*pointer` to supply the pointed-to value. An ordinary C varargs function alone cannot discover argument types. Preserve a separate, explicit printf formatting mode.

See [formatting design](BLACKBOARD_FORMATTING.md) for the refined user-preferred syntax, example dispatch and test requirements.

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
