#include "analyzer/analyzer.hpp"
#include "common/context.hpp"
#include "common/diagnostics.hpp"
#include "lexer/lexer.hpp"
#include "lowering/lowering.hpp"
#include "parser/parser.hpp"
#include "vm/vm.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[]) {
  bool quiet = true;

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [file]" << std::endl;
    return 1;
  }
  if (argc > 2 && std::strcmp(argv[2], "-v") == 0) {
    quiet = false;
  }

  std::string source, buf;

  std::ifstream file(argv[1]);

  while (std::getline(file, buf)) {
    source += buf;
    source += "\n";
  }

  if (!quiet) {
    std::cout << source;
  }

  CompilerContext ctx({.filename = argv[1], .sourceCode = source});
  Lexer lexer(source, ctx);

  auto lexerResult = lexer.tokenize();

  if (!quiet) {
    for (auto token : lexerResult) {
      std::cout << token_to_string(token) << std::endl;
    }
  }

  if (!ctx.errors.empty()) {
    printErrors(ctx);
    return 1;
  }

  Parser parser(lexerResult, ctx);
  std::shared_ptr<ASTNode> parserResult;
  try {
    parserResult = parser.parse();

  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
    printErrors(ctx);
    return 1;
  }

  if (!quiet) {
    printAST(parserResult);
  }

  Analyzer analyzer(ctx);

  analyzer.analyze(parserResult);

  if (!ctx.errors.empty()) {
    printErrors(ctx);
    return 1;
  }

  IRGenerator generator(parserResult, ctx);
  auto IR = generator.generate();

  if (!quiet) {
    for (auto instr : IR) {
      std::cout << IRInstr_to_string(instr) << std::endl;
    }

    std::cout << "top level variable mapping: \n";
    for (auto np : ctx.variableNamespaces) {
      for (auto [k, v] : np) {
        std::cout << k << " -> " << v << std::endl;
      }
    }
  }

  if (!ctx.errors.empty()) {
    printErrors(ctx);
    return 1;
  }

  VMContext vmctx{.ctx = ctx};
  VirtualMachine vm = VirtualMachine(IR, vmctx);
  vm.run();

  return 0;
}
