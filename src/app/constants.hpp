#pragma once

#include <string_view>

namespace AppConstants {
inline constexpr std::string_view Version = "0.1.0";

inline constexpr std::string_view Help =
R"(
Usage: {0} <run file | eval code | (stdin)> [options]

Commands:
  run <file>        Compile and run a source file.
  eval <code>       Compile and run code passed as an argument.
  (no subcommand)   Read a program from standard input.

Options:
  --help            Show this help.
  --version         Print the version.
  --tokens          Print the lexer tokens.
  --ast             Print the parsed AST.
  --ir              Print the generated IR instructions.
  --check           Compile and check, but do not run.
  --O0              Disable optimizations.
  --O1              Enable optimizations.
  --                Pass everything after it to the program.

Examples:
  {0} run examples/01_hello_world.mylang
  {0} eval 'print(add(1, 2));'
  echo 'print("hi");' | {0}
  {0} run example.mylang --tokens --ast --ir
  {0} run example.mylang --check
)";
} // namespace AppConstants
