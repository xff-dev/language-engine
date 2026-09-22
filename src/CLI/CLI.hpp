#pragma once
#include <string>
#include <string_view>
#include <vector>

struct CliOptions {
  bool help = false;
  bool version = false;
  bool interactive = false;

  bool printTokens = false;
  bool printAst = false;
  bool printIR = false;
  bool checkSyntax = false;

  std::string evalCode;
  std::string inputFile;

  std::vector<std::string> programArgs;

  std::string programName;

  int optimization = 0;
};

class CLI {
public:
  CLI(int argc, char **argv);
  CliOptions parse();

private:
  void parseFlag(CliOptions &options, std::string_view flag);

private:
  int argc;
  char **argv;
};
