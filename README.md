# MineCompiler

A tiny C-like programming language written in C++20 with a full compilation
pipeline — lexer, parser, semantic analyzer, IR lowering, and a virtual-machine
interpreter.

## Features

- **C-like syntax** — variables, integer and string values, arithmetic builtins
  (`add`, `sub`, `mul`, `div`), `if`/`eq`/`not`, `while` loops, and recursive
  `func`tions with `return`.
- **Full pipeline** — source goes through a lexer, a recursive-descent parser
  (with source locations and diagnostics), a semantic analyzer, IR lowering,
  and interpretation on a virtual machine with builtins.
- **CLI** — three modes: `run <file>`, `eval <code>`, and reading a program from
  stdin when no subcommand is given.
- **Pipeline dumps** — `--tokens`, `--ast`, and `--ir` print each stage;
  `--check` performs compile-time checks without running the program.
- **Tested** — Catch2 unit tests cover every stage, plus a smoke-test script
  that runs all `examples/*.mylang` end to end.

## Requirements

- C++20 compiler (GCC/Clang)
- CMake >= 3.14
- make
- Python 3 (for the example smoke tests)

## Building

Catch2 is fetched automatically via CMake's `FetchContent`. Building produces
two executables:

- `build/basic` — the language driver.
- `build/unit_tests` — the Catch2 test suite.

### With project-manager (pm) — recommended

The project is driven through
[project-manager](https://github.com/xff-dev/ProjectManager) (`pm`). Register it
once, then build:

```bash
pm add MineCompiler $(pwd)   # register once
pm build                     # build the current project
```

`pm build` reads the `[build]` section of `project.ini`: it creates `build/`,
runs the one-time `cmake ..` prepare step, and then `make -j12`. Prefix the
whole "build and run" workflow so:

```bash
pm build run                 # build, then run the demo example
pm list-scripts              # see the available named scripts
pm script test               # run the unit tests
pm script test_examples      # run the example smoke tests
```

### Manually

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Usage

```
Usage: basic <run file | eval code | (stdin)> [options]
```

| Mode              | Description                          |
| ----------------- | ------------------------------------ |
| `run <file>`      | Compile and run a source file.       |
| `eval <code>`     | Compile and run code from the CLI.   |
| *(no subcommand)* | Read a program from stdin.           |

| Flag          | Description                             |
| ------------- | --------------------------------------- |
| `--help`      | Show help.                              |
| `--version`   | Print the version.                      |
| `--tokens`    | Print the lexer output.                 |
| `--ast`       | Print the parsed AST.                   |
| `--ir`        | Print the generated IR instructions.    |
| `--check`     | Lex, parse and analyze, but don't run.  |
| `--O0`/`--O1` | Select the optimization level.          |
| `--`          | Pass everything after it to the program.|

### Examples

```bash
./build/basic run examples/01_hello_world.mylang          # run a file
./build/basic eval 'print(add(1, 2));'                    # run a one-liner
printf 'print("hi");\n' | ./build/basic                   # run from stdin
./build/basic run example.mylang --tokens --ast --ir      # inspect the pipeline
./build/basic run example.mylang --check                  # check, don't run
```

### Language tour

Values are dynamically typed and are either integers or strings.

Variables and printing:

```c
x = 5;
print(x);
```

Arithmetic is done through builtins (`add`, `sub`, `mul`, `div`); `add` also
concatenates strings:

```c
print(add(10, 20));
print(sub(a, b));
print(mul(a, b));
print(div(a, b));
message = add("Hello, ", "Alex");
```

Conditionals via `if`, `eq`, and `not`; `0` and `""` are falsy:

```c
if (eq(x, 10)) {
  print("x equals 10");
}

if (not(eq(x, 5))) {
  print("x is not 5");
}
```

Loops:

```c
i = 0;
while (sub(i, 5)) {
  print(i);
  i = add(i, 1);
}
```

Functions with parameters, `return`, and recursion:

```c
func factorial(n) {
  if (eq(n, 0)) {
    return 1;
  }

  return mul(n, factorial(sub(n, 1)));
}

print(factorial(5));
```

Function parameters shadow global variables in their own scope. See the
`examples/` directory for many more sample programs.

## Testing

```bash
# unit tests (Catch2) for every pipeline stage
./build/unit_tests

# smoke-test: run every examples/*.mylang end to end
python3 scripts/check_examples.py
```

Or through pm (see `pm list-scripts`):

```bash
pm script test               # ./build/unit_tests
pm script test_examples      # python3 scripts/check_examples.py
```

## Project layout

```
src/
  CLI/       Argument parsing: subcommands, flags, CliOptions
  app/       App layer: runs the pipeline, prints dumps and errors
  common/    CompilerContext and error diagnostics
  lexer/     Tokenizer and token types
  parser/    Recursive-descent parser, AST, source locations
  analyzer/  Semantic analysis pass
  lowering/  IR instruction set and code generation
  vm/        Virtual-machine interpreter and builtins (print, add, ...)
  main.cpp   Entry point: parse the CLI and delegate to App
tests/       Catch2 suite, one file per stage
examples/    Sample .mylang programs
scripts/     Helper scripts (check_examples.py example smoke tests)
project.ini  project-manager build/run/script configuration
```

## AI usage

This project is developed with AI assistance. See [AI.md](AI.md) for details.