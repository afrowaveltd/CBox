# Blackboard Console — proposed CBox module

**Status:** design / learning proposal, **not implemented**. Do not present the proposed names below as existing public APIs. The first actual parser is a learner-owned C17 exercise.

## Purpose

Build a very small Spectre.Console-like ANSI/terminal formatting module for the CBox **Blackboard** family. Shared *terminology and observable markup behavior* with the .NET Afrowave Toolbox are goals where suitable, but **no .NET runtime, ncurses, or DoesItFail dependency** is required by production Blackboard.

Current CBox (at the time of this document) contains `essentials` and `when_it_fails` foundations; a `blackboard/` directory is not yet committed.

## Placement and dependencies

```text
CBox/
    blackboard/
        include/cbox/blackboard/console.h     # Proposed public API
        src/console.c                        # Proposed simple writer and parser
        src/ansi.c                           # Proposed style/ANSI encoder
        src/terminal_platform.c              # Proposed optional platform adapter
        tests/                              # Learner tests
        docs/                               # Module documentation
```

Only the C17 standard library is required for the platform-independent text-parsing core. Terminal detection and control need a **small optional platform adapter** (such as POSIX `isatty` or the Windows Console API); platform-dependent code must not leak into the parser. Production CBox and Blackboard must **not** link DoesItFail. Only Blackboard's test executables may use it.

## Public API naming — decision to settle before implementation

CBox's existing convention requires exported names to start `cbox_`, so a proposed portable API is:

```c
/* Proposed signatures; NOT implemented. */
typedef struct cbox_blackboard_console cbox_blackboard_console;

int cbox_blackboard_write(cbox_blackboard_console *console, const char *markup);
int cbox_blackboard_writeline(cbox_blackboard_console *console, const char *markup);
int cbox_blackboard_clear_screen(cbox_blackboard_console *console);
int cbox_blackboard_reset_style(cbox_blackboard_console *console);
```

Return contracts, error types, and console construction/destruction remain open design decisions. The user's preferred compact spellings `console_write`, `console_writeline`, and `clean_screen` are useful for a tiny stand-alone exercise; for the public library prefer **`clear_screen`** as the common English terminal operation name and the namespace-safe `cbox_` prefix. Optional short aliases, if desired, must be explicitly opted into to avoid collisions with other C libraries.

A console context can hold the target output stream (`FILE *`), ANSI enabled flag, color capability, and a base style/theme; **the output stream is borrowed**, not closed by Blackboard. Avoid hidden function-local `static` mutable output state so multiple independent consoles can coexist.

## Stage 1: streaming markup parser

Target examples:

```c
/* Conceptual syntax; final API may use an explicit context. */
console_writeline("Text [blue]blue[/] text");
console_write("[underline][red]Very important[/][/] normal");
```

Read bytes of a NUL-terminated string sequentially and recognize ASCII markup tag syntax without copying the entire input. UTF-8 text between tags can pass through as unchanged bytes; correct **display width** and cursor navigation require further Unicode-aware functionality and belong to later stages.

Initial supported tags:

| Markup | ANSI effect (if supported) |
| --- | --- |
| `[red]` | SGR foreground red, e.g. `ESC[31m` |
| `[blue]` | SGR foreground blue, e.g. `ESC[34m` |
| `[underline]` | SGR underline on, e.g. `ESC[4m` |
| `[/]` | Restore prior style from a stack, or base style when closing outermost tag |

**Important:** `[/]` is not simply `ESC[0m` if tags are nested. Example `[red]a[underline]b[/]c[/]` must show `c` as red (not underlined). A tiny bounded style stack will support the specified nesting depth; return a defined error if it is exceeded. For an initial learning milestone, implement just one color and closing tags first, then add nested style behavior.

The low-level writer should set or restore **both foreground and background** as needed when applying a complete style, rather than assuming that the terminal has a particular default theme. `ESC[0m` resets the terminal's rendition attributes to its defaults, **not** to a guaranteed black/white background. Provide an explicit theme with contrast-tested color pairs for later use, and leave unaltered terminal defaults when a requested color capability is unavailable.

## Stage 2: terminal controls

- `clear_screen`: clear the visible screen then move the cursor home; typical VT sequences are `ESC[2J` and `ESC[H`. Do **not** use `system("clear")` or modify shell history/scrollback by default.
- Cursor position and hide/show come later, with explicit capability checks.
- For redirection to files/pipes, write **plain text** and do not emit screen-clear, cursor moves, or other terminal-control bytes by default.
- Respect a user choice to disable color (`NO_COLOR`) and degrade gracefully for `TERM=dumb` / unsupported terminals.
- On Windows console handles, VT output may require enabling `ENABLE_VIRTUAL_TERMINAL_PROCESSING` through `GetConsoleMode` / `SetConsoleMode`. Keep this in the platform adapter; do not assume all stdout handles are consoles.
- Color depth (basic 8/16, 256 or truecolor) and whether backgrounds can be reliably inferred are **capabilities**, not universal constants.

## Stage 3: robustness and escape rules

- Escaped brackets: consider Spectre-style `[[` → `[` and `]]` → `]`.
- Unknown tags, unmatched closing tags and truncated tags need a documented policy (literal output vs explicit parse error); decide it first and test it.
- Treat input as untrusted. Markup tags must be parsed by a whitelist. Escape data passed as plain text to avoid interpreting user input as control syntax; prevent injection of raw ESC bytes when appropriate.
- A writer must leave the terminal in a reasonable style state after success **and after failure**; if it changes a shared stream's style, restore the baseline on exit where possible.
- Use `fwrite` of contiguous text spans rather than allocating copies of every character or calling `printf` once per byte. Avoid silently performing heap allocations for every output call.
- Do not assume a NUL terminator for raw binary streams; a future `write_n(ptr, len)` can handle bounded buffer input.

## Named placeholders are separate from markup

`{count}` in an ordinary C string **cannot** automatically reference a local C variable by its name. A future API must take explicit typed arguments, a named map, or a safe format mechanism. Do not overload markup parsing with auto-reflection that C does not provide. Also distinguish named placeholders from C's `printf` format syntax to prevent format-string vulnerabilities.

## Testing strategy and milestones

**M0 (learner's first exercise):** implement `console_write(const char *)` for `[red]`, `[blue]`, and `[/]` on an ANSI-capable test terminal. One parser loop, no production dependencies and no heap allocation.

**M1:** add `console_writeline`, tests with captured output, escaping, and handling of missing or unknown tags. Test output with ANSI enabled and disabled.

**M2:** nested styles with a bounded stack (`[underline][red]...[/]...[/]`), foreground+background styles and robust restoration.

**M3:** explicit output/context object, terminal capability detection, color disable and platform adapters.

**M4:** cursor and clear-screen commands, theme/contrast controls, Windows testing, redirection behavior.

**M5:** explicit named substitutions, later panels/tables if real consumers require them. Full ncurses-like keyboard input and terminal widgets are **out of scope** for the lightweight core.

Some useful tests: no markup, repeated tags, nested tags, escaped brackets, unknown tag, missing closing bracket, unclosed style, empty string, long text, ASCII tags embedded in UTF-8 content, stream write failure, no-color mode, redirected output and background readability.

## Original reference code inspected

- Spectre.Console open-source repository: https://github.com/spectreconsole/spectre.console
- `src/Spectre.Console.Ansi/AnsiMarkup.cs`: tokenization, tag interpretation, style and link stacks.
- `src/Spectre.Console.Ansi/AnsiMarkupTagParser.cs`: interpreting named style/color tags.
- `src/Spectre.Console.Ansi/AnsiWriter.cs`: separation of writer from parser and emitting VT control sequences.
- `src/Spectre.Console.Ansi/AnsiDetector.cs`: output redirection and capability/platform detection.
- Markup reference: https://spectreconsole.net/console/reference/markup-reference/
- Windows VT mode: https://learn.microsoft.com/en-us/windows/console/setconsolemode

Study concepts and behavior; write our own teaching implementation. Spectre.Console is MIT-licensed, but any direct copying requires observance of the upstream license and attribution terms. Do not promise identical styling behavior where terminal support differs.
