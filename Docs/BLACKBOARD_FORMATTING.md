# Blackboard — Formatting and named placeholder design

**Status:** architecture proposal only; there is no implemented CConsole formatting API yet. User goal is to support both idiomatic C `printf` placeholders (e.g. `%d`) and convenient **named** placeholders (e.g. `{apples_count}`) together with Spectre-like style markup.

## Example use cases

```c
int apples = 15;
int *apples_count = &apples;

/* Mode 1: native printf-style conversions in a markup format. */
cbox_console_writeline("[blue]You have %d apples[/]", *apples_count);

/* Mode 2: explicit, typed named arguments. API shape is illustrative. */
cbox_console_writeline_named(
    "[blue]You have {apples_count} apples[/]",
    CBOX_INT("apples_count", *apples_count)
);

/* Future optional name-stringifying macro (still explicit binding): */
cbox_console_writeline_named(
    "[blue]You have {apples_count} apples[/]",
    CBOX_BIND_INT(apples_count, *apples_count)
);
```

All signatures and macros here are **drafts, not committed APIs**. In the code above `apples_count` must be a valid non-NULL `int *`; unary `*` **dereferences** the pointer, producing the `int` required by `%d`. Passing `apples_count` itself to `%d` is a type mismatch; `%p` with `(void *)apples_count` displays the pointer value (address), not the pointed-to integer.

## No hidden C variable lookup

C17 cannot look up a local variable's address, type or contents by name from an arbitrary runtime string. Therefore:

```c
cbox_console_writeline("[blue]You have {*apples_count} apples[/]");
```

must **not** secretly try to dereference a name read from the markup source: the library does not know the compiler's local symbol table. A `*name` fragment inside a format string is not C's unary dereference operator. If named syntax is eventually allowed to include a star as a presentational feature, it may only select **explicitly bound typed data**, never perform pointer arithmetic/dereference on opaque raw addresses.

Prefer `{apples_count}` with `CBOX_INT("apples_count", *apples_count)`; optional macros could stringify an identifier using C preprocessor `#` while evaluating a separate expression. C11 `_Generic` might simplify type selection, but must not bypass lifetime/type validation.

## Two independently understandable formatting front ends

1. **Printf-style markup formatter** accepts a compile-time format string with standard C `printf` conversions (document exact subset/version before release) and normal C varargs. Aim for GCC/Clang compile-time `format(printf,...)` checks when supported, without making compiler extensions mandatory in C17. Consider a `va_list` variant for wrapper authors.
2. **Named typed formatter** receives an explicit collection of name/type/value bindings; for instance integers, floating-point values, borrowed UTF-8 strings and possibly safe pointer display values. Define what happens for missing or duplicate names, invalid UTF-8, unsupported conversions and large output.
3. **Literal text writer** never parses markup or placeholders, useful for filesystem entries, AJIS/JSON values and other untrusted content.

It is acceptable that the **printf-style** and **named-style** entry points have separate names in ordinary C; C does not support native function overloading. A convenience macro dispatcher is optional only after API behavior is settled.

## Crucial security and composition contract

**Only the trusted format template is allowed to define markup. Values inserted through `%s` or named arguments must be treated as literal data**, not retroactively reparsed as style tags or action names. Example: the value `"[Error]foo"` must print as those literal characters inside the surrounding `[blue]` style; it must not turn the next output red.

Do **not** blindly `vsnprintf` the entire markup template and then parse the formatted result: that incorrectly interprets injected `[`...`]` sequences. The actual formatting pipeline has to preserve the boundary between trusted markup and substituted literal data, even if that requires a bounded temporary buffer or a more complex formatter. This is a deliberate exception to the no-allocation common case if necessary for safety and compatibility.

- `printf` formats must come from trusted application code, not arbitrary untrusted text; `%n` and other potentially dangerous/irrelevant conversions need an explicit policy.
- Define how `%%`, `%*d` (dynamic width/precision semantics, **not** C pointer dereference), positional specs if supported, null strings, encoding, embedded newlines and floating-point locale interact with style spans.
- Use a consistent error status for missing bindings and invalid format. Do not invoke undefined behavior by guessing a varargs type from `%d`.
- Prefer span-based input/output: formatting input is borrowed, supplied string values are borrowed for the call, output is streamed or buffered transiently. No permanent ownership transfer and no durable copy of the finished formatted line.
- During live rendering, emitted segments must still support width measurement, wrapping, clipping and click hit regions; formatted values must be accounted for in layout.

## Proposed test matrix

- `[blue]You have %d apples[/]` with `15`; pointer dereferenced by caller vs pointer address.
- Named `{apples_count}` bound to integer; no magic lookup; repeated placeholder names.
- User string `"[Error]x[/]"` is emitted literally, never parsed as markup.
- Nested styles around formatted values; literal braces, `%%`, unknown names and malformed markup.
- Correct C format flags, widths, precision and errors to the documented support level.
- Unicode strings and emoji in values with accurate final width; large values and bounded allocations.
- Plain/ANSI/recording output; redirected output contains no escape sequences.
- Diagnostics under ASan/UBSan when possible; compiler format warnings for supported toolchains.

Related: [themes and semantic markup](BLACKBOARD_THEMES_AND_INIT.md), [console architecture](BLACKBOARD_CONSOLE_DESIGN.md).
