#include "analyzer.hpp"
#include "../vm/builtins.hpp"
#include <format>
#include <memory>
#include <stdexcept>
#include <string>

template <typename ExpectedType>
std::shared_ptr<ExpectedType>
assert_node_type(const std::shared_ptr<ASTNode> &node) {
  auto casted = std::dynamic_pointer_cast<ExpectedType>(node);
  if (!casted) {
    throw std::logic_error("unexpected node type");
  }
  return casted;
}

Analyzer::Analyzer(CompilerContext &ctx) : ctx(ctx) {}

void Analyzer::analyze(const std::shared_ptr<ASTNode> &root) {
  parseFunctions(assert_node_type<BlockNode>(root));

  pushVariableNamespace();
  visit(root);
  popVariableNamespace();
}

void Analyzer::visit(const std::shared_ptr<ASTNode> &node) {
  if (auto blockNode = std::dynamic_pointer_cast<BlockNode>(node)) {
    visitBlock(blockNode);
  } else if (auto returnNode = std::dynamic_pointer_cast<ReturnNode>(node)) {
    visitReturn(returnNode);
  } else if (auto ifNode = std::dynamic_pointer_cast<IfNode>(node)) {
    visitIf(ifNode);
  } else if (auto whileNode = std::dynamic_pointer_cast<WhileNode>(node)) {
    visitWhile(whileNode);
  } else if (auto equalNode = std::dynamic_pointer_cast<EqualNode>(node)) {
    visitEqual(equalNode);
  } else if (auto functionNode =
                 std::dynamic_pointer_cast<FunctionNode>(node)) {
    visitFunction(functionNode);
  } else if (auto callNode = std::dynamic_pointer_cast<CallNode>(node)) {
    visitCall(callNode);
  } else if (auto variableNode =
                 std::dynamic_pointer_cast<VariableNode>(node)) {
    visitVariable(variableNode);
  }
}

void Analyzer::visitBlock(const std::shared_ptr<BlockNode> &node) {
  for (auto childNode : node->statements) {
    visit(childNode);
  }
}

void Analyzer::visitReturn(const std::shared_ptr<ReturnNode> &node) {
  if (functionDepth == 0) {
    ctx.errors.push_back({
        .type = CompilerErrorType::ReturnOutsideFunction,
        .message = "return outside a function",
        .start = node->location,
        .end = node->location,
    });
  }
  if (node->value) {
    visit(node->value);
  }
}

void Analyzer::visitIf(const std::shared_ptr<IfNode> &node) {
  visit(node->condition);
  visit(node->body);
}

void Analyzer::visitWhile(const std::shared_ptr<WhileNode> &node) {
  visit(node->condition);
  visit(node->body);
}

void Analyzer::visitEqual(const std::shared_ptr<EqualNode> &node) {
  visit(node->value);
  createVariable(node->variable);
}

void Analyzer::visitFunction(const std::shared_ptr<FunctionNode> &node) {
  pushVariableNamespace();
  functionDepth++;
  for (auto arg : node->params) {
    createVariable(arg);
  }
  visit(node->body);
  functionDepth--;
  popVariableNamespace();
}

void Analyzer::visitCall(const std::shared_ptr<CallNode> &node) {
  if (!isFunction(node->call)) {
    ctx.errors.push_back({
        .type = CompilerErrorType::UndefinedFunction,
        .message = std::format("undefined function {}", node->call),
        .start = node->location,
        .end = node->location,
    });
  } else if (!isFunctionCallValid(node->call, node->args.size())) {
    ctx.errors.push_back({
        .type = CompilerErrorType::FunctionArgumentMismatch,
        .message =
            std::format("function argument mismatch in {}()", node->call),
        .start = node->location,
        .end = node->location,
    });
  }
  for (auto childNode : node->args) {
    visit(childNode);
  }
}

void Analyzer::visitVariable(const std::shared_ptr<VariableNode> &node) {
  if (!isVariable(node->value)) {
    ctx.errors.push_back({
        .type = CompilerErrorType::UndefinedVariable,
        .message = std::format("undefined variable {}", node->value),
        .start = node->location,
        .end = node->location,
    });
  }
}

void Analyzer::parseFunctions(const std::shared_ptr<BlockNode> &root) {
  for (auto node : root->statements) {
    if (auto functionNode = std::dynamic_pointer_cast<FunctionNode>(node)) {
      if (isFunction(functionNode->name)) {
        ctx.errors.push_back({
            .type = CompilerErrorType::Redeclaration,
            .message = "function redefinition",
            .start = functionNode->location,
            .end = functionNode->location,
        });
      } else {
        functions[functionNode->name] = functionNode->params.size();

        std::set<std::string> uniqueParams;
        for (auto param : functionNode->params) {
          if (uniqueParams.contains(param)) {
            ctx.errors.push_back({
                .type = CompilerErrorType::Redeclaration,
                .message = std::format("redefinition of a parameter {}", param),
                .start = functionNode->location,
                .end = functionNode->location,
            });
          }
          uniqueParams.insert(param);
        }
      }
      parseFunctions(assert_node_type<BlockNode>(functionNode->body));
    } else if (auto ifNode = std::dynamic_pointer_cast<IfNode>(node)) {
      parseFunctions(assert_node_type<BlockNode>(ifNode->body));
    } else if (auto whileNode = std::dynamic_pointer_cast<WhileNode>(node)) {
      parseFunctions(assert_node_type<BlockNode>(whileNode->body));
    }
  }
}

bool Analyzer::isFunction(const std::string &name) {
  if (builtins::isBuiltin(name)) {
    return true;
  }
  return functions.find(name) != functions.end();
}

bool Analyzer::isVariable(const std::string &name) {
  for (int i = (int)variables.size() - 1; i >= 0; i--) {
    const auto &vars = variables[i];

    if (vars.contains(name)) {
      return true;
    }
  }
  return false;
}

void Analyzer::pushVariableNamespace() { variables.push_back({}); }
void Analyzer::popVariableNamespace() { variables.pop_back(); }

void Analyzer::createVariable(const std::string &name) {
  variables.back().insert(name);
}

bool Analyzer::isFunctionCallValid(const std::string &name, int argn) {
  if (builtins::isBuiltin(name)) {
    return builtins::checkArgLen(name, argn);
  } else {
    return functions[name] == argn;
  }
}
