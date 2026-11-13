# WhenItFails

Reserved for a **higher-level** native-C counterpart to [Toolbox WhenItFails](https://github.com/afrowaveltd/Toolbox/tree/master/WhenItFails).

It will eventually cover cross-module failure handling or diagnostics **after** the minimal Essentials Result and Response contracts are stable.

## Dependency direction

```text
when_it_fails -> essentials
```

Never introduce the reverse dependency.

## Planned layout

- `include/cbox/when_it_fails/`: public C headers
- `src/`: implementation files
- `tests/`: module tests
- `docs/`: usage and behavior documentation

**Nothing in this module is implemented yet.** Placeholders only.
