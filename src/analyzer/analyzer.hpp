#pragma once

#include "../common/context.hpp"
#include "../parser/ASTnodes.hpp"
#include <memory>
#include <set>
#include <string>
#include <vector>

class Analyzer {
public:
  Analyzer(CompilerContext &ctx);

  void analyze(const std::shared_ptr<ASTNode> &root);

private:
  CompilerContext &ctx;

  std::vector<std::set<std::string>> variables;
  std::unordered_map<std::string, int> functions;

  void parseFunctions(const std::shared_ptr<BlockNode> &root);

  void visit(const std::shared_ptr<ASTNode> &node);

  void visitBlock(const std::shared_ptr<BlockNode> &node);
  void visitReturn(const std::shared_ptr<ReturnNode> &node);
  void visitIf(const std::shared_ptr<IfNode> &node);
  void visitWhile(const std::shared_ptr<WhileNode> &node);
  void visitEqual(const std::shared_ptr<EqualNode> &node);
  void visitFunction(const std::shared_ptr<FunctionNode> &node);
  void visitCall(const std::shared_ptr<CallNode> &node);
  void visitVariable(const std::shared_ptr<VariableNode> &node);

  bool isFunction(const std::string &name);
  bool isVariable(const std::string &name);

  bool isFunctionCallValid(const std::string &name, int argn);

  void pushVariableNamespace();
  void popVariableNamespace();

  void createVariable(const std::string &name);

  int functionDepth = 0;
};
