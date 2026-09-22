#include "CLI.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

CLI::CLI(int argc, char **argv) : argc(argc), argv(argv) {}

CliOptions CLI::parse() {
  CliOptions options;

  int reservedArgc = 0;

  if (argc >= 2 && std::string(argv[1]) == "run") {
    if (argc < 3)
      throw std::runtime_error("Expected filename");

    reservedArgc = 3;
    options.inputFile = argv[2];
  } else if (argc >= 2 && std::string(argv[1]) == "eval") {
    if (argc < 3)
      throw std::runtime_error("Expected code to evaluate");

    reservedArgc = 3;
    options.evalCode = argv[2];
  } else {
    options.interactive = true;
    reservedArgc = 1;
  }

  bool passArguments = false;

  for (int i = reservedArgc; i < argc; i++) {
    if (!passArguments) {
      if (std::string(argv[i]) == "--") {
        passArguments = true;
        continue;
      }

      if (argv[i][0] != '-') {
        throw std::runtime_error("unexpected argument " + std::string(argv[i]));
      }

      parseFlag(options, argv[i]);
    } else {
      options.programArgs.push_back(argv[i]);
    }
  }

  return options;
}

void CLI::parseFlag(CliOptions &options, std::string_view flag) {
  if (flag[0] != '-')
    return;

  if (flag == "--help")
    options.help = true;
  else if (flag == "--version")
    options.version = true;

  else if (flag == "--tokens")
    options.printTokens = true;
  else if (flag == "--ast")
    options.printAst = true;
  else if (flag == "--ir")
    options.printIR = true;
  else if (flag == "--check")
    options.checkSyntax = true;

  else if (flag == "--O0")
    options.optimization = 0;
  else if (flag == "--O1")
    options.optimization = 1;
  else
    throw std::runtime_error("unexpected flag: " + std::string(flag));
}
