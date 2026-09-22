# AI usage in this project

This project is developed with help from AI coding assistants (LLMs). AI is
used in several distinct ways, and the codebase is structured to make
AI-assisted work practical: each pipeline stage lives in its own module, so a
single stage or the whole source tree can be handed to an assistant as context.

## Tests

The vast majority of the test suite (`tests/`) is written by AI, based on
discussions of the expected behaviour and edge cases: tokenization, parse
errors and source locations, analyzer diagnostics, IR emission, and VM
semantics. `scripts/check_examples.py` — the example smoke-test runner — is also
AI-drafted. All tests are manually reviewed and run (`./build/unit_tests`,
`python3 scripts/check_examples.py`) before changes are accepted.

## Documentation

Documentation — the `README.md`, this file, and the AI-usage notes — is written
by AI from descriptions of the project, its commands, the pipeline stages, and
its structure, then reviewed for accuracy.

## Application code (`src/`)

The code under `src/` is written by hand and maintained manually. AI is used as
a consultant and reviewer for:

- architecture questions (module boundaries, how a new feature fits into the
  lexer → parser → analyzer → IR → VM pipeline);
- implementation questions (idiomatic C++20 usage, standard-library choices,
  error handling and diagnostics patterns);
- review feedback on proposed code and debugging assistance.

AI does not generate the `src/` code wholesale; changes are proposed in
discussion, reviewed, and written by hand.

## Usage policy

- All code changes are reviewed and understood before being committed.
- AI-generated code may be modified or rewritten.
- AI is used as a tool, not as an autonomous author.