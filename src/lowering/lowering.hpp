#include "../common/context.hpp"
#include "../parser/ASTnodes.hpp"
#include "ir.hpp"
#include <memory>
#include <string>
#include <vector>

class IRGenerator {
public:
  IRGenerator(std::shared_ptr<ASTNode> root, CompilerContext &ctx);

  std::vector<IRInstr> generate();

private:
  std::shared_ptr<ASTNode> root;
  CompilerContext &ctx;

  std::vector<IRInstr> instructions;

  int tempCounter = 0;
  int labelCounter = 0;

  std::string newTemp();
  std::string newLabel(std::string name = "");

  std::string getVariable(std::string var);

  void visit(const std::shared_ptr<ASTNode> &node);

  void visitBlock(const std::shared_ptr<BlockNode> &node);
  void visitReturn(const std::shared_ptr<ReturnNode> &node);
  void visitIf(const std::shared_ptr<IfNode> &node);
  void visitWhile(const std::shared_ptr<WhileNode> &node);
  void visitEqual(const std::shared_ptr<EqualNode> &node);
  void visitFunction(const std::shared_ptr<FunctionNode> &node);

  std::string visitExpr(const std::shared_ptr<ASTNode> &node);

  std::string visitNumber(const std::shared_ptr<NumberNode> &node);
  std::string visitString(const std::shared_ptr<StringNode> &node);
  std::string visitVariable(const std::shared_ptr<VariableNode> &node);

  std::string visitCall(const std::shared_ptr<CallNode> &node);
};
