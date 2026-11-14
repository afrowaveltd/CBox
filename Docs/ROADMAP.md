# CBox roadmap

This is a proposed **learning and implementation order**, not a release promise.

## Phase 0 — Repository foundation

- [x] Repository and MIT license created
- [x] Module boundaries and dependency direction documented
- [x] Folder skeleton and placeholders committed
- [x] Basic `.gitignore`, `.editorconfig`, and Allman `.clang-format`
- [x] C17 smoke-test target and initial documentation
- [x] Confirm fresh Debian 13 clone and `make help` / `make test` (2026-11-13; GCC/cc C17 smoke test passed on SLAVEK-LIN)

## Phase 1 — Essentials: Result

- [ ] Compare Toolbox `Result` behavior with desired small C API
- [ ] Decide status and error-description representation
- [ ] Document ownership/lifetime and invalid-input contracts
- [ ] Write unit tests and first language-neutral test scenarios
- [ ] Implement Result in C17 without external runtime dependencies
- [ ] Use Result in a real MemView operation

## Phase 2 — Essentials: Response

- [ ] Decide how typed payloads are represented in C
- [ ] Establish empty/success/failure and payload ownership semantics
- [ ] Test payload lifetime, invalid states, copying, and cleanup
- [ ] Implement minimal Response and usage examples
- [ ] Use Response in MemView

## Phase 3 — WhenItFails

- [ ] Define responsibilities that do not belong in Essentials
- [ ] Reuse Essentials rather than introducing reverse dependencies
- [ ] Align error scenarios with Toolbox where appropriate
- [ ] Test independent builds and MemView integration

## Phase 4 — Cross-language interoperability at the behavioral level

- [ ] Shared specification schema and versioning
- [ ] Reusable fixtures for CBox and Toolbox
- [ ] Consider AJIS conformance tests where meaningful
- [ ] Add other language implementations only when maintainers want them

**Non-goals for foundation:** port every .NET module, promise a stable ABI, or create a heavyweight framework before one useful type has been proven.

## Parallel development tool — DoesItFail (independent repository)

- [x] Choose independent library architecture and development-only dependency direction
- [x] Prepare opt-in CBox static-library integration and version-controlled VS Code tasks
- [x] Document integration, C17 portability policy, and AJIS/AjisFS exploratory notes
- [ ] Publish DoesItFail source as its own GitHub repository (separate starter package prepared)
- [ ] Verify complete first Result implementation and commit learner-owned `result.c`
- [ ] Gradually migrate Result tests from plain `assert` to DoesItFail EXPECT checks
- [ ] Add more assertion types and optional ncurses reporter **in DoesItFail**, not CBox
- [ ] Introduce packaging/install rules once the public API is stable
