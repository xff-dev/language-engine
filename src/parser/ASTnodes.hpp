#pragma once

#include "../common/diagnostics.hpp"
#include <memory>
#include <string>
#include <vector>
struct ASTNode {
  SourceLocation location;
  virtual ~ASTNode() = default;
};

struct NumberNode : ASTNode {
  std::string value;
};

struct StringNode : ASTNode {
  std::string value;
};

struct VariableNode : ASTNode {
  std::string value;
};

struct CallNode : ASTNode {
  std::string call;
  std::vector<std::shared_ptr<ASTNode>> args;
};

struct BlockNode : ASTNode {
  std::vector<std::shared_ptr<ASTNode>> statements;
};

struct IfNode : ASTNode {
  std::shared_ptr<ASTNode> condition;
  std::shared_ptr<ASTNode> body;
};

struct WhileNode : ASTNode {
  std::shared_ptr<ASTNode> condition;
  std::shared_ptr<ASTNode> body;
};

struct FunctionNode : ASTNode {
  std::string name;
  std::vector<std::string> params;
  std::shared_ptr<ASTNode> body;
};

struct ReturnNode : ASTNode {
  std::shared_ptr<ASTNode> value;
};

struct EqualNode : ASTNode {
  std::string variable;
  std::shared_ptr<ASTNode> value;
};
