#include "diagnostics.hpp"
#include "../parser/ASTnodes.hpp"
#include "context.hpp"
#include <format>
#include <iostream>

std::string_view to_string(TokenType type) {
  switch (type) {
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::Number:
    return "Number";
  case TokenType::String:
    return "String";
  case TokenType::LParen:
    return "LParen";
  case TokenType::RParen:
    return "RParen";
  case TokenType::LBrace:
    return "LBrace";
  case TokenType::RBrace:
    return "RBrace";
  case TokenType::Comma:
    return "Comma";
  case TokenType::Semicolon:
    return "Semicolon";
  case TokenType::KeywordFunc:
    return "KeywordFunc";
  case TokenType::KeywordIf:
    return "KeywordIf";
  case TokenType::KeywordWhile:
    return "KeywordWhile";
  case TokenType::EndOfFile:
    return "EndOfFile";
  case TokenType::Equal:
    return "Equal";
  case TokenType::KeywordReturn:
    return "Return";
  default:
    return "Unknown";
  }
}

std::string token_to_string(const Token &token, bool verbose) {
  if (verbose) {
    return std::format("[{}:{}] {}(\"{}\")", token.location.line,
                       token.location.column, to_string(token.type),
                       token.value);
  } else {
    return std::format("{} \"{}\"", to_string(token.type), token.value);
  }
}

std::string_view to_string(IROp op) {
  switch (op) {
  case IROp::Const:
    return "Const";
  case IROp::Mov:
    return "Mov";
  case IROp::Call:
    return "Call";
  case IROp::Label:
    return "Label";
  case IROp::Jmp:
    return "Jmp";
  case IROp::JmpIf:
    return "JmpIf";
  case IROp::JmpIfNot:
    return "JmpIfNot";
  case IROp::FuncBegin:
    return "FuncBegin";
  case IROp::FuncEnd:
    return "FuncEnd";
  case IROp::Return:
    return "Return";
  default:
    return "UnknownIR";
  }
}

std::string IRInstr_to_string(const IRInstr &instr) {
  using std::format;

  auto join_args = [&](const std::vector<std::string> &args) {
    std::string out;
    for (size_t i = 0; i < args.size(); ++i) {
      if (i)
        out += ", ";
      out += args[i];
    }
    return out;
  };

  switch (instr.instr) {
  case IROp::Const:
    return format("{} = {} {}", instr.result, to_string(instr.instr),
                  instr.value);
  case IROp::Mov:
    return format("{} = {} {}", instr.result, to_string(instr.instr),
                  instr.value);
  case IROp::Call: {
    auto a = join_args(instr.args);
    return format("{} = {} {}({})", instr.result, to_string(instr.instr),
                  instr.value, a);
  }
  case IROp::Label:
    return format(":{}", instr.value);
  case IROp::Jmp:
    return format("{} {}", to_string(instr.instr), instr.value);
  case IROp::JmpIf:
    return format("{} {} -> {}", to_string(instr.instr),
                  instr.args.empty() ? "" : instr.args[0], instr.value);
  case IROp::JmpIfNot:
    return format("{} {} -> {}", to_string(instr.instr),
                  instr.args.empty() ? "" : instr.args[0], instr.value);
  case IROp::FuncBegin:
    return format("{} {} ({})", to_string(instr.instr), instr.value,
                  join_args(instr.args));
  case IROp::FuncEnd:
    return format("{} {}", to_string(instr.instr), instr.value);
  case IROp::Return:
    if (instr.args.empty())
      return std::string("Return");
    return format("Return {}", instr.args[0]);
  default:
    return format("UnknownInstr");
  }
}

void printAST(const std::shared_ptr<ASTNode> &node, int depth) {
  if (!node)
    return;

  std::string indent(depth * 2, ' ');
  auto location =
      std::format("[{}:{}] ", node->location.line, node->location.column);

  if (auto n = std::dynamic_pointer_cast<NumberNode>(node)) {
    std::cout << indent << location << "NumberNode: " << n->value << "\n";
  } else if (auto n = std::dynamic_pointer_cast<StringNode>(node)) {
    std::cout << indent << location << "StringNode: " << n->value << "\n";
  } else if (auto n = std::dynamic_pointer_cast<VariableNode>(node)) {
    std::cout << indent << location << "VariableNode: " << n->value << "\n";
  } else if (auto n = std::dynamic_pointer_cast<EqualNode>(node)) {
    std::cout << indent << location << "EqualNode: " << n->variable << " =\n";
    printAST(n->value, depth + 1);

  } else if (auto n = std::dynamic_pointer_cast<CallNode>(node)) {
    std::cout << indent << location << "CallNode: " << n->call << "\n";
    for (const auto &arg : n->args) {
      printAST(arg, depth + 1);
    }
  } else if (auto n = std::dynamic_pointer_cast<BlockNode>(node)) {
    std::cout << indent << location << "BlockNode:\n";
    for (const auto &stmt : n->statements) {
      printAST(stmt, depth + 1);
    }
  } else if (auto n = std::dynamic_pointer_cast<IfNode>(node)) {
    std::cout << indent << location << "IfNode:\n";
    std::cout << indent << "  Condition:\n";
    printAST(n->condition, depth + 2);
    std::cout << indent << "  Body:\n";
    printAST(n->body, depth + 2);
  } else if (auto n = std::dynamic_pointer_cast<WhileNode>(node)) {
    std::cout << indent << location << "WhileNode:\n";
    std::cout << indent << "  Condition:\n";
    printAST(n->condition, depth + 2);
    std::cout << indent << "  Body:\n";
    printAST(n->body, depth + 2);
  } else if (auto n = std::dynamic_pointer_cast<FunctionNode>(node)) {
    std::cout << indent << location << "FunctionNode: " << n->name << " (";
    for (size_t i = 0; i < n->params.size(); ++i) {
      std::cout << n->params[i] << (i + 1 < n->params.size() ? ", " : "");
    }
    std::cout << ")\n";
    printAST(n->body, depth + 1);
  } else if (auto n = std::dynamic_pointer_cast<ReturnNode>(node)) {
    std::cout << indent << location << "ReturnNode:\n";
    if (n->value) {
      printAST(n->value, depth + 1);
    } else {
      std::cout << indent << "  void\n";
    }
  }
}

void printErrors(CompilerContext &ctx) {
  for (const auto &error : ctx.errors) {
    std::cout << ctx.filename << ":" << error.start.line << ":"
              << error.start.column << " error: " << error.message << "\n\n";

    std::vector<std::string> lines;
    std::string currentLine;
    for (char c : ctx.sourceCode) {
      if (c == '\n') {
        lines.push_back(currentLine);
        currentLine.clear();
      } else {
        currentLine += c;
      }
    }
    if (!currentLine.empty()) {
      lines.push_back(currentLine);
    }

    int errorLineIdx = error.start.line - 1;
    if (errorLineIdx >= 0 && errorLineIdx < static_cast<int>(lines.size())) {
      std::cout << "    " << lines[errorLineIdx] << "\n";
      std::cout << "    ";
      for (int i = 1; i < error.start.column; ++i) {
        std::cout << " ";
      }
      std::cout << "^\n";
    }

    std::cout << "\n";
  }
}
