#include "../../src/CLI/CLI.hpp"
#include "catch2/catch_test_macros.hpp"

#include <vector>

static CliOptions parseCommandLine(std::vector<const char *> args) {
  std::vector<char *> argv;
  argv.reserve(args.size());
  for (auto arg : args)
    argv.push_back(const_cast<char *>(arg));

  CLI cli(static_cast<int>(argv.size()), argv.data());
  return cli.parse();
}

TEST_CASE("CLI - run command sets input file", "[cli][parse]") {
  auto options = parseCommandLine({"mine", "run", "example.mylang"});

  REQUIRE(options.inputFile == "example.mylang");
  CHECK(options.interactive == false);
  CHECK(options.evalCode.empty());
}

TEST_CASE("CLI - eval command sets eval code", "[cli][parse]") {
  auto options = parseCommandLine({"mine", "eval", "1 + 2"});

  REQUIRE(options.evalCode == "1 + 2");
  CHECK(options.interactive == false);
  CHECK(options.inputFile.empty());
}

TEST_CASE("CLI - no command enables interactive mode", "[cli][parse]") {
  auto options = parseCommandLine({"mine"});

  REQUIRE(options.interactive == true);
  CHECK(options.inputFile.empty());
  CHECK(options.evalCode.empty());
}

TEST_CASE("CLI - flags are recognized before program arguments", "[cli][parse]") {
  auto options = parseCommandLine({"mine", "run", "example.mylang", "--check", "--tokens", "--ast", "--ir", "--O1", "--", "--user", "value"});

  REQUIRE(options.inputFile == "example.mylang");
  CHECK(options.checkSyntax == true);
  CHECK(options.printTokens == true);
  CHECK(options.printAst == true);
  CHECK(options.printIR == true);
  CHECK(options.optimization == 1);
  REQUIRE(options.programArgs.size() == 2);
  CHECK(options.programArgs[0] == "--user");
  CHECK(options.programArgs[1] == "value");
}

TEST_CASE("CLI - help and version flags can be combined", "[cli][parse]") {
  auto options = parseCommandLine({"mine", "--help", "--version"});

  REQUIRE(options.help == true);
  CHECK(options.version == true);
  CHECK(options.interactive == true);
}
