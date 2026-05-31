#include "lowering.hpp"
#include "ir.hpp"
#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

IRGenerator::IRGenerator(std::shared_ptr<ASTNode> root, CompilerContext &ctx)
    : root(root), ctx(ctx) {};

std::vector<IRInstr> IRGenerator::generate() {
  instructions.clear();
  ctx.variableNamespaces.push_back({});
  visit(root);

  return instructions;
}

template <typename T>
std::shared_ptr<T> expect_node(const std::shared_ptr<ASTNode> &node) {
  auto casted = std::dynamic_pointer_cast<T>(node);
  if (!casted) {
    throw std::logic_error("AST invariant violated");
  }
  return casted;
}

void IRGenerator::visit(const std::shared_ptr<ASTNode> &node) {
  if (auto equalNode = std::dynamic_pointer_cast<EqualNode>(node)) {
    visitEqual(equalNode);
  } else if (auto blockNode = std::dynamic_pointer_cast<BlockNode>(node)) {
    visitBlock(blockNode);
  } else if (auto callNode = std::dynamic_pointer_cast<CallNode>(node)) {
    visitCall(callNode);
  } else if (auto returnNode = std::dynamic_pointer_cast<ReturnNode>(node)) {
    visitReturn(returnNode);
  } else if (auto functionNode =
                 std::dynamic_pointer_cast<FunctionNode>(node)) {
    visitFunction(functionNode);
  } else if (auto ifNode = std::dynamic_pointer_cast<IfNode>(node)) {
    visitIf(ifNode);
  } else if (auto whileNode = std::dynamic_pointer_cast<WhileNode>(node)) {
    visitWhile(whileNode);
  } else {
    visitExpr(node);
  }
}

std::string IRGenerator::visitExpr(const std::shared_ptr<ASTNode> &node) {
  if (auto stringNode = std::dynamic_pointer_cast<StringNode>(node)) {
    return visitString(stringNode);
  } else if (auto numberNode = std::dynamic_pointer_cast<NumberNode>(node)) {
    return visitNumber(numberNode);
  } else if (auto variableNode =
                 std::dynamic_pointer_cast<VariableNode>(node)) {
    return visitVariable(variableNode);
  } else if (auto callNode = std::dynamic_pointer_cast<CallNode>(node)) {
    return visitCall(callNode);
  }
  return "error";
}

void IRGenerator::visitFunction(const std::shared_ptr<FunctionNode> &node) {
  ctx.variableNamespaces.push_back({});
  std::vector<std::string> args;
  for (auto param : node->params) {
    std::string temp = newTemp();
    ctx.variableNamespaces.back()[param] = temp;
    args.push_back(temp);
  }

  instructions.push_back({
      .instr = IROp::FuncBegin,
      .value = node->name,
      .args = args,
  });

  visitBlock(expect_node<BlockNode>(node->body));
  ctx.variableNamespaces.pop_back();
  instructions.push_back({.instr = IROp::FuncEnd, .value = node->name});
}

void IRGenerator::visitBlock(const std::shared_ptr<BlockNode> &node) {

  for (auto childNode : node->statements) {
    visit(childNode);
  }
}

void IRGenerator::visitReturn(const std::shared_ptr<ReturnNode> &node) {
  if (node->value) {
    std::string value = visitExpr(node->value);
    instructions.push_back({
        .instr = IROp::Return,
        .args = {value},
    });
  } else {
    instructions.push_back({
        .instr = IROp::Return,
    });
  }
}

std::string IRGenerator::getVariable(std::string var) {
  std::string temp;
  size_t namespacesSize = ctx.variableNamespaces.size();

  for (int i = namespacesSize - 1; i >= 0; i--) {
    auto &variableNamespace = ctx.variableNamespaces[i];
    if (variableNamespace.find(var) != variableNamespace.end()) {
      temp = variableNamespace[var];
      return temp;
    }
  }
  temp = newTemp() + "_" + var;
  ctx.variableNamespaces.back()[var] = temp;

  return temp;
}

void IRGenerator::visitEqual(const std::shared_ptr<EqualNode> &node) {
  std::string var = getVariable(node->variable);
  std::string value = visitExpr(node->value);

  instructions.push_back({
      .instr = IROp::Mov,
      .result = var,
      .value = value,
  });
}

std::string IRGenerator::visitString(const std::shared_ptr<StringNode> &node) {
  std::string var = newTemp();
  instructions.push_back({
      .instr = IROp::Const,
      .result = var,
      .value = node->value,
  });

  return var;
}

std::string IRGenerator::visitNumber(const std::shared_ptr<NumberNode> &node) {
  std::string var = newTemp();
  instructions.push_back({
      .instr = IROp::Const,
      .result = var,
      .value = node->value,
  });

  return var;
}

void IRGenerator::visitIf(const std::shared_ptr<IfNode> &node) {
  std::string condition = visitExpr(node->condition);
  std::string elseLabel = newLabel("else");

  instructions.push_back({
      .instr = IROp::JmpIfNot,
      .value = elseLabel,
      .args = {condition},
  });

  visitBlock(expect_node<BlockNode>(node->body));

  instructions.push_back({
      .instr = IROp::Label,
      .value = elseLabel,
  });
}

void IRGenerator::visitWhile(const std::shared_ptr<WhileNode> &node) {
  std::string elseLabel = newLabel("else");

  std::string startLabel = newLabel("while");

  instructions.push_back({
      .instr = IROp::Label,
      .value = startLabel,
  });

  std::string condition = visitExpr(node->condition);
  instructions.push_back({
      .instr = IROp::JmpIfNot,
      .value = elseLabel,
      .args = {condition},
  });

  visitBlock(expect_node<BlockNode>(node->body));

  instructions.push_back({
      .instr = IROp::Jmp,
      .value = startLabel,
  });

  instructions.push_back({
      .instr = IROp::Label,
      .value = elseLabel,
  });
}

std::string
IRGenerator::visitVariable(const std::shared_ptr<VariableNode> &node) {
  std::string var = getVariable(node->value);
  return var;
}

std::string IRGenerator::visitCall(const std::shared_ptr<CallNode> &node) {
  std::string var = newTemp();

  std::vector<std::string> args;

  for (auto arg : node->args) {
    args.push_back(visitExpr(arg));
  }

  instructions.push_back({
      .instr = IROp::Call,
      .result = var,
      .value = node->call,
      .args = args,
  });

  return var;
}

std::string IRGenerator::newTemp() { return std::format("t{}", tempCounter++); }

std::string IRGenerator::newLabel(std::string name) {
  if (!name.empty()) {
    return std::format("l{}_{}", labelCounter++, name);
  } else {
    return std::format("l{}", labelCounter++);
  }
}
