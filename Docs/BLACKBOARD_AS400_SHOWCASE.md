# Blackboard — AS/400-inspired showcase and theme

**Status:** planned demo and built-in theme; **not** a working IBM terminal emulator, and not an actual IBM i / 5250 connection. This example is intended as an affectionate, practical showcase of CConsole's tables, themed fields and keyboard/mouse focus.

## Showcase intent

- Add an optional **`as400` theme preset**, selectable via programmatic API and/or theme INI, in addition to the modern built-in dark default.
- Add an **`as400_demo`** example using CConsole's own widgets, not a bespoke framebuffer renderer. This tests interoperability across table, layout, focus, footer/status row and resizing.
- Render a nostalgic text-based menu on an 80-column screen (original IBM 5250 devices commonly used 24 x 80; some wide-screen models supported 27 x 132). Offer a monochrome green-on-near-black and an optional multicolor variant.
- Use normal CConsole interaction: Tab/Shift+Tab to focus, Space to activate, optional mouse click, and configurable function-key shortcuts (F3 Exit, F5 Refresh). These are **our demo controls**, not a claim of full 5250 keyboard emulation.
- Default desktop ANSI backend retains modern UTF-8 capability; choose ASCII glyphs and fallback borders when rendering in true legacy text consoles.
- Status/command footers demonstrate fixed-position panels. Highlight full active rows using theme foreground/background, including spaces; they must remain readable in monochrome and on terminals without reliable background color.

### Illustrative appearance

```text
AFROWAVE CCONSOLE                              SYSTEM: CBOX

                C O N S O L E   D E M O

OPT     DESCRIPTION                      STATUS
 1      Markup and Colors                READY
 2      Tables and Panels                READY
 3      Theme Manager                    READY
>4      Interactive Actions              SELECTED
 5      Progress and Status              READY

F3=Exit   F5=Refresh   Tab=Next   Space=Select
```

**Theme sketch** (exact palette awaits accessibility/contrast tests):

```ini
[theme]
name = AS400
background = #06170E
foreground = #B4F0C4

[roles]
info.foreground = #A0F5B0
warning.foreground = #E3D477
error.foreground = #FF8D8D
success.foreground = #63EB8F
focus.background = #C6DF92
focus.foreground = #102218
```

Use **the same layout/rendering and terminal backends** as every other theme. This showcase is a compatibility test and UX demonstration, not a second UI architecture or additional runtime dependency.

Reference: IBM i device/emulation mappings including monochrome 24x80 IBM 5251 and wide-screen 27x132 IBM 3180/3477:
https://www.ibm.com/docs/en/i/7.5.0?topic=problems-troubleshooting-emulation-types
