#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "app.hpp"
#include "constants.hpp"

#include "../analyzer/analyzer.hpp"
#include "../common/context.hpp"
#include "../lexer/lexer.hpp"
#include "../lowering/lowering.hpp"
#include "../parser/parser.hpp"
#include "../vm/vm.hpp"

App::App(CliOptions &options) : options(options) {}

int App::run() {
  if (options.help)
    return printHelp();
  if (options.version)
    return printVersion();
  if (options.interactive)
    return runInteractive();
  if (!options.evalCode.empty())
    return runString(options.evalCode);

  return runFile(options.inputFile);
}

int App::printHelp() {
  std::cout << std::format(AppConstants::Help, options.programName)
            << std::endl;
  return 0;
}

int App::printVersion() {
  std::cout << AppConstants::Version << std::endl;
  return 0;
}

int App::runString(std::string_view source) {
  CompilerContext ctx;

  ctx.sourceCode = source;
  ctx.filename = "[STDIN]";

  auto IR = compileCtx(ctx, options);

  if (IR == std::nullopt)
    return 1;

  if (!options.checkSyntax) {
    VMContext vmctx{.ctx = ctx};
    VirtualMachine vm = VirtualMachine(IR.value(), vmctx);
    vm.run();
  }

  return 0;
}

int App::runInteractive() {
  std::string source, buf;

  while (std::getline(std::cin, buf)) {
    source += buf;
  }
  return App::runString(source);
}

int App::runFile(std::string_view filename) {
  CompilerContext ctx;

  if (!options.inputFile.empty()) {
    std::string source, buf;
    std::ifstream file(options.inputFile);

    while (std::getline(file, buf)) {
      source += buf;
      source += "\n";
    }
    ctx.sourceCode = source;
    ctx.filename = options.inputFile;
  }

  auto IR = compileCtx(ctx, options);

  if (IR == std::nullopt)
    return 1;

  if (!options.checkSyntax) {
    VMContext vmctx{.ctx = ctx};
    VirtualMachine vm = VirtualMachine(IR.value(), vmctx);
    vm.run();
  }

  return 0;
}

std::optional<std::vector<IRInstr>> compileCtx(CompilerContext &ctx,
                                               CliOptions &options) {
  Lexer lexer(ctx.sourceCode, ctx);

  auto lexerResult = lexer.tokenize();

  if (options.printTokens) {
    for (auto token : lexerResult) {
      std::cout << token_to_string(token) << std::endl;
    }
  }

  if (!ctx.errors.empty()) {
    printErrors(ctx);
    return std::nullopt;
  }

  Parser parser(lexerResult, ctx);
  std::shared_ptr<ASTNode> parserResult;
  try {
    parserResult = parser.parse();

  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
    printErrors(ctx);
    return std::nullopt;
  }

  if (options.printAst) {
    printAST(parserResult);
  }

  Analyzer analyzer(ctx);

  analyzer.analyze(parserResult);

  if (!ctx.errors.empty()) {
    printErrors(ctx);
    return std::nullopt;
  }

  IRGenerator generator(parserResult, ctx);
  auto IR = generator.generate();

  if (options.printIR) {
    for (auto instr : IR) {
      std::cout << IRInstr_to_string(instr) << std::endl;
    }
  }

  if (!ctx.errors.empty()) {
    printErrors(ctx);
    return std::nullopt;
  }

  return IR;
}
