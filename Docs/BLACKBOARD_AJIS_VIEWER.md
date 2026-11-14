# Blackboard structured-data viewer — AJIS + JSON

**Status:** proposed component and dependency contract, no native C17 AJIS/JSON parser or Blackboard viewer implementation is claimed.

## Product idea

Blackboard provides a **single interactive structured-data viewer** for **AJIS** and **strict JSON**, backed by the eventual AJIS parser's supported dialects. The viewer should work as a stand-alone terminal app, an embeddable renderable and a FileBrowser preview provider.

Spectre.Console reference: the separate `Spectre.Console.Json` extension includes `JsonText`, `IJsonParser`, JSON syntax classes and a style-aware JSON renderable (see https://github.com/spectreconsole/spectre.console/tree/main/src/Extensions/Spectre.Console.Json). Our aim is a **superset of display use cases**, not an unsupported promise of identical parsing semantics or API compatibility.

## Dependency rule

```text
FileBrowser / CLI tools
         |
         v
Blackboard AJIS/JSON Viewer
        / \
       v   v
Blackboard render/layout/input   AJIS parser public API
      |                              |
      v                              v
Terminal backends              Input streams/bytes
```

The AJIS parser MUST NOT depend on Blackboard or any terminal API. Blackboard's rendering engine MUST NOT depend on AJIS. Viewer is an **optional integration module** that consumes both. Keep the AJIS parser's implementation ownership/repository choice separate from this viewer design; do not silently place parser code inside the console framework.

## Structured-data features

- Colored syntax: keys, strings, numbers, booleans, null, braces, brackets, commas, AJIS directives, pointers and annotations, as available from a validated parser token/event model.
- Tree navigation: collapse/expand object and array nodes; keyboard navigation; configurable indentation; breadcrumb/path at the cursor.
- Strict JSON mode must accept only valid JSON; AJIS-specific syntax (comments, number prefixes, directives, etc.) MUST NOT accidentally become valid JSON merely because the AJIS parser understands it.
- AJIS mode can expose AJIS-specific information where supported: pointers/references, version directives, optional attachments and payload metadata. Do not assert AJIS 2.0 binary/AjisFS formats are currently stable or implemented in C.
- Go to path/key, find values, jump between matches, optionally copy a path/value into a supported terminal clipboard integration only when explicitly requested.
- Toggle between structured tree and syntax-colored source view; offer plain-text fallback for logs/redirection and unsupported terminals.
- Clear, location-aware parse error diagnostics (offset, line/column when available), with no crashes on corrupt or truncated data.
- FileBrowser should select this preview provider for recognized AJIS/JSON content; do not infer format solely from extension when content detection is available.
- No automatic editing or rewriting of source files in the first viewer. Viewer must be read-only by default.

## Large files and low-memory operation

- Prefer **token/event streaming** or lazy navigation over building a full object tree for every file.
- A streaming parser can support sequential syntax highlighting, but interactive expand/collapse and random jumps require a **seekable source, sparse index, cached ranges, or another explicit navigation strategy**. Do not promise arbitrary backward navigation in a nonseekable stream without caching.
- For a local file, retain byte offsets and sparse indexes where parser contracts permit; cache only nearby material or visible nodes. Allow a bounded disk cache if justified.
- Avoid rendering the entire document; render the visible viewport based on terminal dimensions. Permit cancellation and safe reopening.
- Define max nesting depth, max token length, payload handling, memory budgets and error states. Never require copying gigabytes into RAM.
- AJIS binary/payload regions should be handled as typed metadata and optional hex/size/offset previews; do not dump arbitrary bytes as text or accidentally parse as markup.

## First learning milestones

1. Verify the AJIS C parser's public event/token contract and strict JSON validation, or define an adapter contract until the parser exists.
2. Create a small fake in-memory token source as a test double — this enables the viewer to be developed without pretending a finished parser exists.
3. Render a colorized JSON example using Blackboard's styled segments and standalone plain output.
4. Add tree navigation and per-node collapse/expand using the shared Blackboard input/layout engine.
5. Connect to the real AJIS parser **only after** its parser API and ownership rules are tested.
6. Add FileBrowser preview integration and large-file pagination/sparse navigation tests.
7. Introduce AJIS-specific features and validate them against known AJIS conformance fixtures.

## Tests

- Same JSON fixture rendered by Spectre.Console.Json (reference) and by Blackboard viewer; compare stable styled text, not raw pixel screenshots.
- JSON strict validity vs AJIS mode using intentionally AJIS-only syntax.
- Nested maps/arrays, escaped strings, Unicode values and large repeated containers.
- Truncated/invalid input, malformed pointers, unknown directives and unsupported format versions.
- File size beyond available RAM, cache eviction, nonseekable input and cancel/reopen behavior.
- ANSI disabled, redirected output, terminal resize and malformed content containing ESC/control bytes.
- The file browser must remain usable even if optional AJIS support is absent or parsing fails.

## Public API notes

No public symbols are locked in yet. Candidate module path: `blackboard/include/cbox/blackboard/structured_viewer.h` with implementation in `blackboard/src/viewers/` or a separate optional `blackboard_ajis` library. Prefer typed callbacks/iterators with explicit context pointers and documented ownership instead of embedding a full parser's internals in the view.

The planned dependency direction lets third-party C applications use the parser without UI, Blackboard without AJIS, or both together for a rich viewer.
