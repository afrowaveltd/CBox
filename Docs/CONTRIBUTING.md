# Contributing to CBox

CBox is developed in small, reviewable steps. Beginners are welcome; prefer clear reasoning and tests over clever shortcuts.

## Learning and implementation flow

1. Define a concrete use case and a small diagnostic question.
2. Explain the underlying C behavior (values, pointers, ownership, lifetime).
3. Agree on the public contract **before** implementing it.
4. Write tests for valid and invalid scenarios.
5. Implement the smallest behavior that passes the tests.
6. Use it in MemView or another real consumer.
7. Update documentation and, when stable, shared specifications.

## Local checks

```bash
make help
make test
```

At the foundation stage `make test` is only a C17 toolchain smoke test. Real module tests will be introduced when code is added.

Use `clang-format` with the repository's `.clang-format` settings for C source. GNU Makefile recipe indentation uses tabs, even though C uses spaces.

## Naming

- Modules and files: `snake_case`.
- Exported C functions/types: `cbox_` prefix (exact future type naming will be reviewed).
- Documentation and public comments: English.
- Portable tests should avoid assuming pointer widths, integer layout beyond the C language guarantees, or particular filesystem paths.

## Commits

Keep commits limited to one coherent step. Do not introduce unused public APIs or claim a feature is tested merely because a compiler smoke test passes.

The initial folders contain `.gitkeep` because Git does not store empty directories. Remove those placeholders as real files arrive.

## License

The repository has an MIT license; preserve its existing notice in derivative distributions as required.

## Keep builds, editor support and docs synchronized

Whenever a new CBox module/test target is added, update the Makefile target, `.vscode/tasks.json`, relevant GDB launch task, include paths and accompanying README/Docs in the **same change**. Use DoesItFail only from test executables; keep its independent implementation in its own repository. `make test-framework` remains optional, while normal smoke tests do not require external test libraries. See [DoesItFail integration](DOES_IT_FAIL_INTEGRATION.md).
