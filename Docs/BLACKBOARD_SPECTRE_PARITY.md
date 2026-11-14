# Blackboard ↔ Spectre.Console feature parity tracker

**Status:** proposed coverage targets only; **none of the Blackboard C features below is implemented or tested yet**. Reference: Spectre.Console documentation (v0.57 at initial review, November 2026), subject to revision as upstream evolves.

Use this tracker to distinguish **semantic parity**, **visual parity**, **API ergonomics** and **platform support**. A green test is evidence of one scenario, not universal pixel-identical behavior.

## Scope of comparison

| Area | Spectre.Console reference capability | Blackboard C17 target | Initial state |
| --- | --- | --- | --- |
| Text/markup | Rich markup, nesting, escape, named/RGB colors, background and decorations | Match grammar and visible behavior; plain mode supported | Planned |
| Styles/theming | Style composition, color depth, foreground/background, hyperlinks | Match where terminal supports; contrast-aware fallback theme | Planned |
| Console/ANSI | VT writer, capabilities, terminal output and control | Portable writer with POSIX/Windows adapter, no mandatory ncurses | Planned |
| Measurement | Styled segments and renderable measurement | Shared cell-width/segment model | Planned |
| Basic renderables | Text, Markup, Rule, alignment/padding | Visually compatible static output | Planned |
| Panel | Borders, title/header, padding, expand | Closely match dimensions and border styles | Planned |
| Table | Automatic widths, rows/headers, borders, wrapping | Rich table builder and width solver | Planned |
| Tree | Nested branches and guides | Equivalent tree layout and Unicode/ASCII fallback | Planned |
| Grid/Columns/Rows | Multi-column composition and flow | Shared reusable layout engine | Planned |
| Layout | Named panels, splits, sizing | Composable, constrained layout graph | Planned |
| Charts/Calendar | Bar charts, breakdown/spark, calendar | Terminal text charts and calendar formats | Planned |
| Progress | Task progress bars and statuses | Multiple tasks, update events and live refresh | Planned |
| Status | Spinners, transient status display | Common styles, safe frame scheduling | Planned |
| Live | In-place renderable updates | Exclusive live session, diff/refresh | Planned |
| Prompts | Text, confirmation, selection and multiselection | Keyboard input abstractions and validation | Planned |
| Input/output redirection | Headless output and interaction checks | Plain text mode and noninteractive behavior | Planned |
| Resize/fallback | Terminal-capability-sensitive behavior | Narrow terminal, resize and non-VT fallback | Planned |
| Extensibility | Custom renderables | C renderable callbacks/descriptors with ownership contract | Planned |
| Testing | TestConsole / renderable verification | Recording backend, snapshot and cross-language comparison tests | Planned |

**Out of scope for the core:** C# generic syntax, reflection, async runtime semantics, C# fluent API source compatibility. These need idiomatic C equivalents, not false equivalence.

## Acceptance tests per feature

- Input and expected visible output example captured using the Spectre.Console C# reference.
- ANSI-enabled output, ANSI-disabled output, and redirected noninteractive output.
- Golden result based on normalized cell grid or styled spans (not a raw terminal screenshot alone).
- Width 80 columns, width 40 columns, narrow/error conditions and truncation policies.
- UTF-8 handling including combining marks/wide characters/emoji; explicitly document unsupported cases.
- Zero invalid memory access under compiler diagnostics and applicable sanitizers.
- Allocation failure and cleanup paths where feature allocates.
- Theme, border and symbol consistency with cross-language fixtures.
- CI platform coverage and tested environment recorded honestly.

## Parity verification method

1. Write equivalent *intended output scenarios* in C# Spectre.Console and in CBox Blackboard.
2. Capture reference styled segments or normalize emitted ANSI to final styled cell grids.
3. Compare style attributes, text, measured widths, wrapping, borders and hierarchy.
4. Where exact output cannot match (terminal font, emoji width ambiguity, unsupported VT feature), document deviations and fallback policies.
5. Keep a snapshot of the upstream Spectre.Console version and relevant documentation. Do not retroactively claim compatibility with features released after that snapshot.

## Target implementation order (not a feature cap)

1. Foundational reference fixtures, output abstraction, ANSI and markup.
2. Common styled-segment and renderable measurement/render pipeline.
3. Text + rule + panel, then table + tree + grid/layout.
4. Live/progress/status with predictable terminal ownership.
5. Prompts and input, plus charts/calendar and extra advanced visualizations.
6. Continuous parity testing, packaging, samples and third-party usage verification.

See [Blackboard architecture and roadmap](BLACKBOARD_CONSOLE_DESIGN.md).