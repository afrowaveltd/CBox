# CBox Blackboard — code display, source formatter and optional syntax highlighting

**Status:** architecture only; no native C17 CodeBlock or language formatter/highlighter implementation exists yet.

## Three independent responsibilities

1. **Original source** is immutable. A CodeBlock can borrow a `const char *` plus length, or read from a bounded source/paged stream. Never modify the caller's source, original file or backing AJISFS data.
2. **Optional language-aware code formatter** (like a VS Code format action) transforms a **working copy** into another valid representation with the chosen style rules (indentation, brace placement, spaces, line breaks). It is a *real source-code formatter*, **not merely display layout**. Rules are per language and configuration (for example C with Allman braces). A correct formatter must respect comments, string literals and syntax; if it cannot safely format, return an error or fall back to the unchanged source. Formatting a whole program is not guaranteed to be single-pass or purely streaming.
3. **Optional syntax highlighter** accepts the **current displayed text** (formatted working copy if formatting ran, otherwise original). It only produces token classifications/style runs, never changes bytes, line breaks or indentation. A highlighter may be absent without preventing formatted/unstyled display.
4. **CodeBlock renderer** handles presentation only (line numbers, gutters, padding, wrap/clip options, scroll, tab display width, terminal capabilities, theme). The renderer must not change the underlying text as a side effect.

```text
                      ORIGINAL SOURCE (immutable)
                                 |
                          [formatter enabled?]
                             /            \
                            yes            no
                            |               |
                  optional working copy    borrowed source
                  (language rule set)      (zero-copy)
                            \               /
                             displayed text view
                                   |
                          [highlighter enabled?]
                             /            \
                          style spans      no spans
                             \            /
                              CodeBlock
                                 |
                  same CConsole terminal renderer
```

**Important:** style-run byte offsets are relative to the **displayed/working text**, not automatically to the original source. If error diagnostics and cursor location need original-file positions, an optional source mapping is a separate concern because formatting changes offsets and line numbers.

## Formatter interface principles

- Formatter returns an explicitly **owned new buffer**, a caller-provided output sink, or a documented borrowed unchanged result. Ownership must be unambiguous: who frees what and whether the returned text aliases input.
- The same formatter logic could later operate over stream chunks using state/checkpoints when syntactically valid. A big source formatter may need multiple passes, temporary spill files or a full syntax tree; never advertise generic safe streaming for every programming language.
- Original indentation, tabs and line endings need a **specified normalization policy for the working copy**. A raw CodeBlock without formatter does not normalize them.
- Language-specific configuration is optional: e.g. C braces=Allman, indent=4, tab policy. Formatter must not invent code transformations that change program semantics.
- Unsupported language, formatter failure, memory budget overflow: display original source or return a documented failure policy. Do not silently truncate source.
- Source presentation formatting is distinct from `CodeBlock`'s visual layout: adding line numbers, horizontal scrolling or painting a background does **not** modify source text.

## Highlighter interface principles

Emit **style metadata** for a byte span in the displayed code, not inserted CConsole `[color]` markup. Code may itself contain `[red]`, `[/]` or terminal control bytes, which must be shown as literal data. The renderer maps token roles through the active theme.

```c
/* Model only, NOT public API yet. */
typedef struct
{
    size_t offset;  /* Into displayed working text */
    size_t length;
    cbox_code_token_role role;
} cbox_code_style_span;
```

A highlighter should not own the entire source when it can borrow/read the displayed view. A language lexer with multiline tokens can carry state between chunks and sparse checkpoints to start at an arbitrary visible line. Reject malformed/out-of-range spans and fall back to plain code on failure.

Inline `[Code]...[/]` remains a simple semantic style, not an arbitrary raw-code container. Raw code should be passed through a literal CodeBlock API or a carefully parsed Markdown fenced-block adapter.

## Large sources and AJISFS / stream sources

- Most small examples can format in a transient working copy and discard it after rendering; don't preserve history unnecessarily.
- For large texts, support explicit limits, paging, line indexes and optional disk-backed temporary working output when needed. Do not allocate the entire file merely to show a small viewport.
- AJISFS-style on-disk content is a potential source provider: request chunks as needed, parse/format or color them and emit progressively, but **lexical/state boundaries must survive chunk splits**. A chunk boundary can land mid-string, mid-comment or mid-token.
- Incremental **display highlighting** is often feasible with preserved lexical context; correct arbitrary **source reformatting** is harder and may require global context. Keep both contracts separate.
- The viewer can always choose an unformatted, uncolored source path when budgets or missing modules make transformation inappropriate.

## Optional module boundaries

- Core terminal renderer/markup: necessary.
- CodeBlock display widget: optional.
- Language-specific formatter: optional, depends on the code-source interface only and must not require a terminal.
- Language-specific highlighter: optional, similarly terminal-independent.
- Markdown/structured-data readers: optional adapters to CodeBlock/widgets or core styled segments. Nothing in core depends on them.
- Initially prefer separate static libraries and a documented registration/interface mechanism. Dynamic libraries/plugin ABI can come later when ownership and versioning are stable.

## Acceptance tests

- Original source bytes are unchanged after success, formatter failure and highlighter failure.
- Formatter output follows selected language settings and remains syntactically/semantically valid (when supported); comments, literals, preprocessing and unusual input tested.
- Working-copy output may differ from original only when formatting was requested.
- Highlighter on/off yields *identical displayed code text* for the same formatting setting.
- Style-run offsets reference the working copy, with Unicode boundaries and wide glyph display handling.
- Fallback to unformatted and/or uncolored text under missing module, unknown language, low memory or malformed input.
- Test very long lines, CRLF, tabs, zero-byte buffers, huge streaming sources, split tokens and lexer-state checkpoints.
- Plain/legacy/ANSI output must not reinterpret code as markup or control sequences.

Related: [Blackboard architecture](BLACKBOARD_CONSOLE_DESIGN.md), [modular builds](BLACKBOARD_MODULAR_BUILD.md), [themes](BLACKBOARD_THEMES_AND_INIT.md).
