# Blackboard — Themes, semantic markup, and initialization

**Status:** design-only proposal, not implemented. This document specifies the user's preferred direction for the native C17 CConsole / CBox Blackboard module and highlights unresolved public API details.

## Goals

- A simple user experience comparable to Spectre.Console plus **portable application-level themes**, useful for installers, diagnostics, test runners and dashboards.
- Default appearance for an explicitly initialized interactive UI session: **very dark gray / nearly black background**, readable foreground and contrasting semantic colors.
- **No forced heavy dependency**, no ncurses requirement, and no retention of old output for ordinary writes. However, memory savings must not remove useful capabilities or make the API needlessly inconvenient.
- Theme files are optional. A compiled-in default always works; users can supply or programmatically override colors and role styles.
- Common semantic names express *meaning* so applications can keep the same source code across themes: `[Info]`, `[Warning]`, `[Error]`, `[Success]`, `[Quote]`, `[Code]`, later `[Muted]`, `[Highlight]`, etc.
- Original explicit Spectre-like color/decoration markup, e.g. `[blue]`, `[bold]`, `[underline]`, and `[/]`, must remain supported. Semantic names extend rather than replace that grammar.

## Distinguish console session from plain output

**Proposed high-level convenience behavior:**

```c
/* Illustrative pseudocode; these functions do NOT yet exist. */
cbox_console_init();  /* Interactive TTY: initialize, apply dark theme, clear screen */
cbox_console_writeline("[Info]Loading...[/]");
cbox_console_writeline("[Error]Unable to load config[/]");
cbox_console_shutdown(); /* Restore terminal mode, cursor, colors where practical */
```

A useful lightweight `init()` should be a convenience facade, not the only way to use the library. Lower-level applications can create explicit independent console instances, write to a provided stream, select a theme and decide whether to clear.

**Semantics to settle before public API freeze:**
1. Check interactive terminal capability before full-screen control. **Never automatically clear a file, pipe, log, or noninteractive stream.** Plain writing should work with no `init()` where feasible.
2. On an interactive console session, initialize once, select theme and set correct foreground/background, then clear and home. Ordering matters: a background-colored *full screen* may require BCE (background color erase support) or writing spaces across the viewport; merely changing SGR background does not paint previously written cells everywhere.
3. Do not assume the user's previous theme was black/white, or that `ESC[0m` restores arbitrary custom palette state. Record any terminal modes changed and restore them on normal cleanup; restore style to the terminal default when ending our own session, but do not promise recovery after hard crashes.
4. Never change the entire user's terminal palette/OS theme by default. Apply theme within **our owned interactive screen**. Optional alternate-screen mode is a separate deliberate feature, not an implicit side effect.
5. Provide explicit flags for **clear-on-init** (default for the high-level interactive TUI facade), **no-clear**, plain mode, and forced/disabled ANSI. Multiple output contexts must not secretly share mutable per-instance state.
6. If there is no user-supplied theme, use the built-in dark theme for our interactive application screen. The passive `write()` mode should **not** repaint the surrounding terminal or unexpectedly change the entire background.

## Proposed INI theme format

This is a readable, low-friction option requiring no third-party parser; final format and syntax will be validated in tests. Do not make the AJIS parser a **runtime dependency** of the Blackboard core just for configuration.

```ini
[theme]
name = Dark
background = #14161A
foreground = #E5E7EB

[roles]
info.foreground = #60A5FA
warning.foreground = #FBBF24
error.foreground = #FB7185
success.foreground = #4ADE80

quote.foreground = #AEB6C2
quote.italic = true

code.foreground = #BEDBFF
code.background = #242A34

focus.foreground = #FFFFFF
focus.background = #24557D
focus.bold = true
```

Config precedence: **compiled default -> optionally loaded config file -> explicit runtime overrides**. Caller explicitly selects config path; avoid silently parsing untrusted current-directory theme files. INI parsing is bounded and documented (line lengths, invalid colors, unknown keys, duplicates, encoding, errors). Unknown roles may be rejected or safely ignored with diagnostics; decide in tests. Missing values fall back to default, and a malformed file must never produce unreadable foreground/background pairs.

The resolved `cbox_theme` is a **small in-memory value/configuration object** reused for all writes. Do not read the INI file or recompute styles for every `console_writeline()`. Themes may be replaced explicitly during an interactive session, triggering necessary redraw of visible content.

## Semantic markup and role resolution

Example:

```c
/* Intended syntax, not a working API. */
cbox_console_writeline("[Info]Checking dependencies[/]");
cbox_console_writeline("[Warning]Disk almost full[/]");
cbox_console_writeline("[Error]Cannot open the file[/]");
cbox_console_writeline("[Quote]Knowledge belongs to everyone[/]");
cbox_console_writeline("[Code]int x = 42;[/]");
```

- Tokenizer identifies the tag name without needing to know its color. The **style resolver** looks up a theme role, and the renderer outputs resolved colors/attributes according to terminal capabilities.
- Maintain a style **stack** so `[bold][Warning]...[/]...[/]` correctly returns to the previous composite style. Specify explicit precedence for nested semantic and literal tags, including foreground/background and decoration inheritance.
- For semantic role names, consider ASCII case-insensitive matching (`[Warning]` / `[warning]`) for convenience, but test and document grammar compatibility instead of silently changing all Spectre markup semantics.
- Semantic roles are presentation by default; interpreting an unknown tag as an action/function is forbidden. Rich `CodeBlock` and `QuoteBlock` **renderables** may later add block layout, wrapping, syntax highlighting and indentation. `[Code]` as an inline tag initially only applies a style; it does **not** imply automatic syntax highlighting or a change in tokenization.
- Raw code must be passed to a CodeBlock renderer independently of markup (source may literally contain `[/]`). A **display formatter** preserves source bytes and indentation; an **optional** highlighter adds token-role style spans over offsets, resolved through the active theme. If no highlighter is installed, CodeBlock is still readable. See [CodeBlock design](BLACKBOARD_CODE_BLOCKS.md).
- The markup entry point is explicitly separate from **literal text writing**. Contents read from file names, AJIS/JSON values and user input are not automatically parsed as markup.
- Converting roles to low-color/monochrome terminals should preserve meaning with optional plain markers (`[!]`, `[ERROR]`) and/or decoration, while keeping output readable.
- Highlight and focus are theme roles for the whole cell span. A selected row must paint/pad all intended cells; no reliance on SGR background alone for unused columns.
- No hidden, automatic shell commands or callback dispatch tied to tag names.

## Optional AS/400-inspired theme and demonstration

Include a built-in `AS400` theme preset with a nostalgic green-on-dark palette and a small interactive **table/menu showcase**, in addition to (not replacing) the modern default dark theme. This is not a full IBM 5250 emulator. The showcase should exercise selection, theme roles, active row background, ASCII/Unicode fallback and status/footer output; no special renderer paths. See [AS/400 showcase](BLACKBOARD_AS400_SHOWCASE.md).

## Text and placeholders

Support **ordinary C printf-style conversions** alongside **explicitly bound typed named placeholders** as distinct formatting front ends, and preserve a pure literal-text writer. C17 cannot discover the address/value of a local `apples_count` from the string `"{*apples_count}"` alone. Most importantly, substituted strings must remain **literal data** and must not become parsed markup/actions. See [Formatting and placeholders](BLACKBOARD_FORMATTING.md).

## Constraints and tests

- Base renderer remains **C17** and standalone; tiny POSIX/Windows adapters provide TTY detection, size, cursor and terminal capabilities. No mandatory ncurses.
- Avoid per-write heap allocations in the common case. A theme holds a small fixed/resolved style table; use streaming or transient render logic except when a live widget needs bounded state.
- Retain existing high-level goal: FileBrowser, AJIS/JSON viewer, tab/pointer focus, mouse and advanced TUI widgets can use exactly the same theme roles.
- Test built-in dark theme, missing config, valid/invalid INI, override precedence, theme reload, color depth fallback, nested semantic+literal markup, style restoration and plain redirection.
- Test initial clear/background on terminals with/without BCE capability, theme init/shutdown without corrupting a caller's terminal and no control codes in redirected output.
- Benchmark theme lookup and write overhead without claiming performance before measurement.

## Open decisions

- Exact API names and signatures: context-based core plus an optional easy `init()/write()/shutdown()` facade.
- Role name casing and how unknown roles are handled.
- Whether the default dark theme is scoped only to interactive full-screen sessions or also to explicit styled inline writes (the safe default is session scope).
- Full-screen background strategy for legacy consoles lacking BCE; when to prefer filling with spaces vs keeping native terminal background.
- Whether INI is the only configuration syntax initially; avoid a dependency on the future native AJIS parser at core level.

Related: [Blackboard overall architecture](BLACKBOARD_CONSOLE_DESIGN.md), [parity tracker](BLACKBOARD_SPECTRE_PARITY.md), [roadmap](ROADMAP.md).
