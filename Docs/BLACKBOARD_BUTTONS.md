# CBox Blackboard / CConsole — Button widget and unified activation

**Status:** architecture proposal only. No Button widget, event loop or C17 input implementation is available yet.

## Objective

Offer a simple, reusable **Button** as a bordered, focusable text element with an optional icon, chosen width, padding, theme styling and a user-defined C activation callback. Mouse activation and keyboard activation dispatch the **same action**, not two different application behaviors. This is a first-class renderable/widget built atop general CConsole action regions; it must not implement a second input subsystem.

## User-visible behavior

- `Tab` moves focus to the next eligible element; `Shift+Tab` moves backward.
- `Space` activates the currently focused Button; `Enter` may also activate. A widget containing editable text must be allowed to consume Space for text entry instead.
- Left mouse click activates the action if enabled and if the terminal/input backend provides mouse events. Mouse is always optional.
- Disabled buttons remain visible but are skipped during focus traversal and cannot be activated. Hidden or clipped widgets do not retain stale hit regions.
- A Button may have a label, optional icon, width/min/max constraints, alignment, border style, horizontal/vertical padding, theme variant and enabled/focused/pressed styles.
- A button's full **painted interactive region** (including internal padding and border, except any defined decorative overlap) can be a hit target. Derive hit bounds from **post-layout screen cell coordinates**, not source UTF-8 character counts.
- The same widget must render usefully in a modern Unicode/RGB terminal, a limited 16-color/ASCII text console and a noninteractive/redirected plain-text stream. Plain output has no clickable semantics.
- Default text/menu focus must remain perceptible in monochrome (e.g., `>`, box/reverse-video fallback) and not be communicated by color alone.

## Illustrative C API, not yet public

```c
/* Pseudocode illustrating API choices, not a compilable public header. */
typedef void (*cbox_console_action_fn)(void *context);

typedef struct
{
    const char *text;           /* Borrowed for widget lifetime or copied by documented option */
    const char *icon;           /* Optional UTF-8 glyph with ASCII fallback */
    size_t width_cells;        /* Requested width in terminal cells; 0 = content width */
    bool enabled;
    cbox_console_action_fn on_activate;
    void *context;             /* Caller-owned; valid until action removed */
} cbox_button_options;

/* Example usage concept: */
cbox_button_options next = {
    .text = "Next",
    .icon = "→",
    .width_cells = 18,
    .enabled = true,
    .on_activate = next_clicked,
    .context = &app
};
```

Avoid freezing the public type before we choose an opaque or value-based widget representation, error model and ownership policy. The callback is invoked from the UI event dispatcher, not immediately from the renderer. Optional future action events may include source (keyboard/mouse), modifiers and widget ID, but do not force complexity on the simplest case.

## Smart defaults and configurable options (design refinement)

Every common widget must be **useful immediately with built-in defaults** and **configurable via a small options structure** without requiring a full builder or custom renderer. Suggested two-level API (illustrative only):

```c
/* One-line convenience: automatic theme/size/alignment */
cbox_console_button("Next", on_next, &app);

/* Detailed configuration */
cbox_button_options options = cbox_button_options_default();
options.text = "Next";
options.icon = "→";
options.width_cells = 20;
options.border = CBOX_BORDER_ROUNDED;
options.content_align = CBOX_ALIGN_CENTER;
options.style_role = CBOX_BUTTON_PRIMARY;
options.on_activate = on_next;
options.context = &app;
cbox_console_button_add(&console, &options);
```

Allow more specialized per-state foreground/background/border/font decorations through an optional style override struct. Default styles resolve through the active theme (normal, focused, hovered, disabled), so a change in theme updates the widget consistently; explicit overrides take precedence where provided.

**Important C17 detail:** zero-initializing `cbox_button_options` alone does **not** represent defaults such as `enabled=true`. Provide an explicit default constructor, initializer macro, or option-presence bitset. A real zero can be a valid user-selected value; do not conflate missing and explicitly configured values. We can consider an `args[]` override list as a later convenience/extension, but a documented options struct with designated fields is easier to understand, type-check and maintain for ordinary C.

**Layout responsibility:** `content_align` decides how icon/label appear inside the button; parent layout parameters decide where the button sits (left/center/right), spacing and flow. Use layout constraints rather than absolute screen coordinates for most applications. Raw coordinates may be supported in specialized canvas/overlay mode.

**Size/ownership:** width and padding are measured in terminal cells, including borders, Unicode symbols and fallback icons. Document whether strings are borrowed/copied and the duration of the borrowed data; do not retain a pointer to an ephemeral stack buffer. Avoid overengineering callbacks with variable argument arrays when `void *context` already allows the caller to pass any structured state.

**Test contract:** default construction + one-line use, options overriding individual defaults, changed theme, disabled/focused/hovered states, width=0 automatic, invalid/out-of-range values, invalid UTF-8, missing icon fallback, and callback context lifecycle.

## Internal event and rendering architecture

```text
terminal adapter (POSIX VT / Win32)
     -> normalized key/mouse event
     -> focus manager / cell-based hit testing
     -> action ID for active Button
     -> callback(context)
     -> application state update
     -> invalidation and redraw
```

**Button is a thin composition:**
- renderable border + optional icon + text + padding
- focusable action region + theme roles
- configured activation callback and context

The **session** owns input mode, event pump, focus graph and redraw orchestration. No per-button terminal reads, `malloc` of full-screen history or duplicate mouse initialization.

### Width, Unicode and legacy fonts

- Width means **terminal cell columns**, not UTF-8 bytes or code points. Count borders, spaces and glyph widths. A wide emoji commonly occupies two cells, but rendering adapts to chosen terminal width policy.
- For a fixed width, define what happens when borders/padding/icon/text do not fit: clip/ellipsis or return a layout warning, never render beyond the button's assigned region. Default width can be content-based.
- Theme carries normal/focused/disabled/pressed colors with legible foreground/background combinations.
- Border and icon symbols have Unicode/ASCII alternatives; e.g. `→` vs `>`, box-drawing vs `+`/`-`/`|`.
- Terminal resizing can reflow button groups; focus identity should survive when its widget still exists.

### Lifetime and errors

- A callback's `context` is caller-owned unless an advanced constructor explicitly transfers ownership. Deregister action regions when widgets/views are destroyed.
- Callback execution must not race with widget destruction or use stale pointers; policy for callback-triggered view replacement and nested event dispatch must be defined.
- Draw-only operations are distinct from an interactive UI session: the same Button may be rendered as a static element without reading keyboard input.
- Test event injection via a recording/headless backend so Space/Tab/click behavior can be verified without a real mouse or terminal.

## Test cases

1. Three buttons: Tab selects the next; Shift+Tab the previous; Space and click invoke the exact same callback.
2. Disabled and hidden buttons cannot be activated and are omitted from focus traversal.
3. A button containing a 2-cell emoji fits or clips correctly at widths 8/16/40; full hit area is correct after resize.
4. Callback invalidates its own view safely; no use-after-free.
5. ANSI 24-bit, 16-color, monochrome/ASCII and redirected output all remain readable.
6. Input mode restoration on normal session end and recoverable errors.

## Suggested implementation order

1. Generic focusable action-region model and injected normalized events.
2. Minimal Button rendering in a fixed rectangle using existing border and text primitives.
3. Keyboard Tab/Shift+Tab/Space activation with a mocked backend.
4. Optional mouse integration and cell-based hit testing.
5. Theme styling, icon fallback, width/alignment and advanced containers.

See [console architecture](BLACKBOARD_CONSOLE_DESIGN.md), [themes](BLACKBOARD_THEMES_AND_INIT.md), [modular builds](BLACKBOARD_MODULAR_BUILD.md).
