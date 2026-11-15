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

## Simpler positional semantics (preferred by the user)

The simplest desired public call is **exactly C-like**, without explicit per-argument binding wrappers:

```c
int apples = 15;
int *apples_count = &apples;

/* Desired CConsole convenience form; not implemented. */
console_writeline_named(
    "[blue]You have {apples_count} apples[/]",
    *apples_count
);

/* Multiple values bind left-to-right, irrespective of label spelling. */
console_writeline_named(
    "[Info]{person} has {number} apples[/]",
    person_name,
    *apples_count
);
```

The identifier inside `{apples_count}` is **a human-readable placeholder label, not a variable lookup key**: first placeholder gets first argument, second gets second, etc. Different names in a format do not affect binding; repeated `{x}` labels consume another positional value each time. A simpler anonymous `{}` placeholder can be supported as well. If labels are allowed, document that they do not validate argument names. Literal braces need a tested escaping rule (candidate `{{`/`}}`). Missing or extra positional arguments should return an error rather than guess.

**C17 implementation mechanism:** `_Generic` from C11 resolves each argument expression's type **at compile time**. Use a macro front-end `console_writeline_named(format, ...)` to wrap each argument in a type-dispatched tagged `cbox_arg` value, then call a real function with a `cbox_arg[]` and count. A normal ellipsis function plus `va_list` **cannot** recover argument types automatically; the macro must generate typed descriptors **before** type erasure. The public macro can be ergonomic even though the internal implementation is explicit. Implement a small preprocessor `FOR_EACH`/arity dispatcher over a documented bounded number of arguments (e.g. start 1–8 or 1–16). A zero-argument/literal fallback is a separate writer API. Make unsupported types a compile error or explicit error policy.

**Sketch, not a complete implementation:**

```c
typedef struct cbox_arg cbox_arg;
cbox_arg cbox_arg_from_int(int value);
cbox_arg cbox_arg_from_double(double value);
cbox_arg cbox_arg_from_text(const char *value);

/* Chooses a constructor by C expression type at compilation. */
#define CBOX_ARG(value) _Generic((value),               \
    int: cbox_arg_from_int,                             \
    float: cbox_arg_from_double,                         \
    double: cbox_arg_from_double,                        \
    char *: cbox_arg_from_text,                          \
    const char *: cbox_arg_from_text                     \
)(value)

/* Public macro would expand CBOX_ARG over all user values.
   A real cbox_console_named_impl receives typed value descriptors. */
```

`_Generic` does not evaluate its controlling expression, so the argument is evaluated only in the selected constructor call. `typeof` is standardized in **C23**, not C17, and `__typeof__` is compiler-specific. We do not need either for basic type dispatch.

**Pointer contract:** `*apples_count` is an `int` expression, so generic dispatch selects integer formatting. Passing `apples_count` is an `int *`, which must **not** be silently dereferenced. Pointer value printing should use an explicit `%p` pathway or a separately documented pointer constructor, rather than unsafe type guessing.

**Format-mode contract:** the classic `printf` entry point remains separate so developers can control conversion, width, precision and base with `%d`, `%.2f`, `%x`, etc. Mixed `%...` and `{...}` in the **same** template need an explicit precedence/argument-consumption policy before being supported, so initial versions can reject the mixture rather than misbind values. Named placeholders choose default presentation by selected C type; optional explicit specifiers might follow later.

**Safety:** inserted `char *` values are always literal text; never reparse them as markup tags or actions. Typed values can be borrowed for the duration of the formatting call and emitted through the existing streaming renderer.

## No hidden C variable lookup

C17 cannot look up a local variable's address, type or contents by name from an arbitrary runtime string. Therefore:

```c
cbox_console_writeline("[blue]You have {*apples_count} apples[/]");
```

must **not** secretly try to dereference a name read from the markup source: the library does not know the compiler's local symbol table. A `*name` fragment inside a format string is not C's unary dereference operator. If named syntax is eventually allowed to include a star as a presentational feature, it may only select **explicitly bound typed data**, never perform pointer arithmetic/dereference on opaque raw addresses.

Prefer `{apples_count}` with `CBOX_INT("apples_count", *apples_count)`; optional macros could stringify an identifier using C preprocessor `#` while evaluating a separate expression. C11 `_Generic` might simplify type selection, but must not bypass lifetime/type validation.

## Two independently understandable formatting front ends

1. **Printf-style markup formatter** accepts a compile-time format string with standard C `printf` conversions (document exact subset/version before release) and normal C varargs. Aim for GCC/Clang compile-time `format(printf,...)` checks when supported, without making compiler extensions mandatory in C17. Consider a `va_list` variant for wrapper authors.
2. **Named/positional typed formatter** receives typed values in left-to-right order, constructed automatically by a C17 `_Generic` convenience macro. An advanced API may also accept an explicit collection of descriptors. Labels inside braces are presentational; names are not used for lookup. Define what happens for missing or duplicate names, invalid UTF-8, unsupported conversions and large output.
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
