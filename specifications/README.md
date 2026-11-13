# Shared behavioral specifications

The long-term goal is to reuse the same conformance scenarios across Afrowave Toolbox (.NET), CBox (C), and possible other language implementations.

Initially this directory has placeholders only. We will define a **versioned, language-neutral schema** after designing and testing the first actual Result API.

Specifications should focus on observable states and expected outcomes, for example successful operations, defined failure cases, empty payloads, and input-validation edge cases. They must not rely on .NET object layouts, C pointer addresses, or implementation-specific memory representation.

`essentials/` and `when_it_fails/` hold the future scenarios for their respective modules.
