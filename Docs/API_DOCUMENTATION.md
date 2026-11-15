# Public API documentation standard

Each exported C function must be documented **next to its declaration in the public header**. Each module should also have a human-readable usage guide under its `docs/` directory.

Doxygen-style comments are recommended because they are readable even without the Doxygen generator.

## Required contract fields

- **Purpose:** what the function does and does not do.
- **Parameters:** type, meaning, valid ranges, nullability, and whether values may be modified.
- **Return:** meaning of each return code or status; whether success can have no data.
- **Failure:** stable error codes, diagnostics, and how the caller can distinguish them.
- **Lifetime:** when input or output objects must remain alive.
- **Ownership:** who allocates, who frees, which function frees, whether ownership transfers.
- **Thread safety:** where relevant; no misleading general promises.
- **Portability:** supported C standard, implementation-dependent assumptions.
- **Example:** compilable minimal use in a relevant module's documentation.
- **Tests:** link to positive, negative, and boundary test scenarios.

## Suggested comment pattern (illustration only)

```c
/**
 * @brief Demonstrates the style for a future API declaration.
 * @param data Pointer to input elements; non-NULL when count is nonzero.
 * @param count Number of readable input elements.
 * @return A documented result describing success or failure.
 * @note The caller retains ownership of data.
 * @warning The caller must keep data alive for the duration of the call.
 */
```

This example intentionally does **not** define a function or a public Result type. Those contracts must be designed and tested first.

## Cross-language cases

Keep language-neutral input/output specifications in `specifications/`. The same scenario can be executed by Toolbox, CBox, and future implementations (RustBox and others). Do not put implementation-dependent pointer addresses or struct memory layouts in shared expected outputs.

## Unix manual reference

For Unix/Linux releases, document implemented exported library entry points in **man(3)**, supporting configuration files in **man(5)**, installed tools in **man(1)**, and module overview in **man(7)**. Man pages are offline quick-reference material that must agree with the public headers, tests and ownership contracts. Do not create callable-looking function man pages before functions actually exist. See [manual page plan](MAN_PAGES.md).

## Review gate

Do not treat a function as ready for general use until its behavior, error semantics, memory contract, unit tests, and example usage are present.
