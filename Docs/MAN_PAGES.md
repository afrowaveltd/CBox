# CBox manual pages (man)

**Status:** manual-page packaging plan plus one real **design-stage** man page; no Blackboard C API is implemented. Man pages for functions or tools will only be published once corresponding features exist and can be tested.

## Why man pages

CBox libraries should support an offline, terminal-native reference in addition to README, English documentation, sample projects, and API comments. Keep these surfaces consistent: short `man` pages for lookups, longer guides for tutorials and rationale, and public header comments for the exact C contract.

## Section conventions

| Section | Intended coverage | Example (planned) |
| --- | --- | --- |
| **7** | CBox/Blackboard architecture and overview | `man 7 cbox-blackboard` |
| **3** | Real public C library functions (only when implemented) | `man 3 cbox_console_init` |
| **5** | Actual supported configuration syntax | `man 5 cbox-theme` |
| **1** | Installed executable tools and demos | `man 1 cconsole-demo` |

In sections 3 and 5/1, names are **provisional illustrations** pending finalized code and executable names. Do not publish placeholder function pages implying that APIs exist.

## Actual source file

The repository contains an English, traditional man macro (roff) source:

```text
man/
  man7/
    cbox-blackboard.7
```

Its `STATUS` section expressly states that CConsole is planned and not callable yet. This file is not currently installed by the Makefile.

On Linux with a man pager installed, preview it directly from a checkout:

```sh
man -l man/man7/cbox-blackboard.7
```

Or use a temporary manual search path (starting from repository root):

```sh
MANPATH="$PWD/man:${MANPATH:-}" man 7 cbox-blackboard
```

System-wide integration will later install sectioned files to `$(mandir)/man7`, `$(mandir)/man3`, etc., respecting `PREFIX`, `DESTDIR`, and the selected module profile. Debian packaging can place them in `/usr/share/man/manN` and optionally compress them through normal packaging processes. Don't hard-code Linux paths into the library itself.

## Documentation contract

A function's man(3) page must include:
- `NAME`, `SYNOPSIS`, `DESCRIPTION`, `PARAMETERS` (where useful);
- `RETURN VALUE`, `ERRORS`, `MEMORY/OWNERSHIP`, `THREAD SAFETY`, `NOTES` / portability;
- `EXAMPLES`, `SEE ALSO`;
- a direct cross-reference to the public declaration and its tested behavior.

For installed optional modules, only install the pages corresponding to shipped features. Keep manual sources in version control. If generating man pages from Doxygen later, compare outputs to real headers and tests; do not allow generated docs to invent signatures.

## Build and test plan

1. Ensure at least `man -l` can render each changed roff page in a Debian test environment; lint with `mandoc -T lint` or other installed validators where available.
2. Add `make man-check` when a suitable optional tool/test path is available; absence of man/groff must not break CBox's ordinary C17 compilation.
3. Add `make install-man` / uninstall or a packaged install rule once the library is actually released, with `DESTDIR` and module selection support.
4. Verify function signatures, return types, ownership semantics and examples against compiled tests whenever an API changes.
5. Keep the manual English as the portable reference; translations may be separate locale-specific man pages later.

See [API documentation contract](API_DOCUMENTATION.md), [Blackboard architecture](BLACKBOARD_CONSOLE_DESIGN.md) and [roadmap](ROADMAP.md).
