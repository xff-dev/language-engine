#pragma once

#include "../CLI/CLI.hpp"
#include "../common/context.hpp"
#include <optional>
#include <string_view>
#include <vector>

class App {
public:
  App(CliOptions &options);

  int run();

private:
  int printHelp();
  int printVersion();
  int runInteractive();
  int runString(std::string_view source);
  int runFile(std::string_view filename);

private:
  CliOptions &options;
};

std::optional<std::vector<IRInstr>> compileCtx(CompilerContext &ctx,
                                               CliOptions &options);
