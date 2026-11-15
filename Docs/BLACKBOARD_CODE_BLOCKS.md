# CBox Blackboard — CodeBlock, display formatter and optional syntax highlighting

**Status:** architecture only. No C17 CodeBlock, code formatter or syntax highlighter has been implemented.

## Goals

- Render code attractively and legibly even without colors or a language-specific highlighter.
- Preserve **original source** as the source of truth (including indentation and line breaks); never change code merely to display it.
- Make syntax highlighting an optional independent module that returns **style metadata** for source spans, not a replacement formatted string.
- Reuse general-purpose CConsole rendering, terminal capabilities, Unicode text measurement, themes and the same minimal-memory principle used by other widgets.

## Responsibilities and data flow

```text
     borrowed original source (UTF-8 bytes + length)
                         |
                         v
             CodeBlock presentation
           /                       \
   base formatter/layout    optional syntax highlighter
   (indentation, lines,      (language lexer, tokens,
    tabs, numbers, clip)      style ranges/roles only)
           \                       /
            \                     /
              theme style resolver
                       |
              CConsole renderable
                       |
        ANSI / legacy / plain terminal output
```

### 1. Original code and CodeBlock formatter

- `CodeBlock` must accept raw literal code with explicit byte length and possibly a read-only source reader for huge inputs.
- Preserve whitespace, blank lines, line order and content. Format **only presentation**: padding/border/background, gutters/line numbers, tab stop calculations and optional scroll indicators.
- For a preformatted block, default to no automatic wrapping: provide horizontal scroll or clip long lines, with a documented opt-in wrap mode that never edits the underlying source.
- Style and control sequences embedded in user code are **never executed as markup or trusted ANSI**. Render raw control characters safely (escaped/visually represented as needed) rather than performing their control actions.
- Work in monochrome/plain-text mode, on narrow terminals, and on redirected output.

### 2. Independent syntax highlighter

- Language recognizer receives only source (or an accessible source window plus lexical context), not the terminal backend or UI control structures.
- It emits ordered **style spans** with offsets and lengths and a stable token role (keyword, string, comment, numeric literal, type, preprocessor directive, etc.).
- CConsole resolves token roles using the current theme. Different themes change colors, never source text.
- Optional module may be absent or fail: CodeBlock draws uncolored code and reports diagnostics only via its normal return/error channel.
- A pure highlighter does not re-indent source, normalize line endings, automatically replace tabs, alter tokens or inject markers.

Illustrative model (subject to contract review):

```c
typedef enum
{
    CBOX_CODE_TOKEN_KEYWORD,
    CBOX_CODE_TOKEN_STRING,
    CBOX_CODE_TOKEN_COMMENT,
    CBOX_CODE_TOKEN_NUMBER,
    CBOX_CODE_TOKEN_TYPE
} cbox_code_token_role;

typedef struct
{
    size_t offset;      /* UTF-8 byte offset into original source */
    size_t length;      /* Original byte span length */
    cbox_code_token_role role;
} cbox_code_style_span;
```

These are **not yet public declarations**.

### Why not insert literal `[color]` tags?

The user's proposed semantics (“keep all characters as-is and add colors”) are right, but generating one concatenated markup string creates problems: the code itself can contain `[blue]`, `[/]`, `[Code]`, or bytes interpreted as terminal escape commands. The source would be reparsed as control syntax rather than displayed literally.

Returning **non-owning style spans over the original buffer** is safer and saves memory. The renderer can traverse original text and style runs together, emitting the right ANSI transitions only at boundaries. It need not allocate a new string containing markup and extra escape sequences.

`[Code]text[/]` remains a quick inline semantic style, but robust raw code blocks use an explicit CodeBlock API. Markdown fenced blocks can delegate their literal contents to it without reparsing them as CConsole markup.

### Memory and streaming

- Initial static CodeBlock may render from a borrowed `const char *` and a bounded line buffer; no persisted highlighted copy.
- For large files, a source adapter can page by byte offsets/line indexes and retain only a visible window. Avoid scanning huge files on every redraw.
- Languages with multiline tokens need saved lexical state and potentially sparse checkpoints at line boundaries; otherwise highlighting a viewport starting in the middle of a block comment can be wrong.
- Streamed style runs are possible; optional bounded caches are acceptable when they improve scroll latency.

### Extension ecosystem

Implement the generic CodeBlock **before** language-specific lexers. Keep highlighters in optional modules (first statically linked; dynamic loading later if useful). Markdown renderer, code preview in FileBrowser and terminal-based documentation browser reuse the same CodeBlock and renderer.

### Test contracts

- Exact source bytes unchanged after CodeBlock/highlighter operation.
- Identical code text/indentation with and without highlighter.
- Source containing markup-like text, ANSI bytes, tabs, CRLF, empty lines and long lines is shown as data.
- Unicode glyph and emoji display width, Unicode style boundaries, clipping and horizontal scrolling.
- Syntax state across line breaks and window boundaries; malformed spans/out-of-range offsets rejected safely.
- Highlighting disabled or unavailable = readable code, not an error that prevents display.
- Low-memory behavior and output parity in ANSI, plain and recording backends.

Related: [Blackboard architecture](BLACKBOARD_CONSOLE_DESIGN.md), [themes](BLACKBOARD_THEMES_AND_INIT.md), [roadmap](ROADMAP.md).
