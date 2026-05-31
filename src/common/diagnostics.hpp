#pragma once
#include "../lexer/token.hpp"
#include "../lowering/ir.hpp"
#include "../parser/ASTnodes.hpp"

#include <memory>
#include <string>
enum class CompilerErrorType {
  UnexpectedToken,
  UnterminatedString,
};

struct CompilerError {
  CompilerErrorType type;
  std::string message;
  SourceLocation start;
  SourceLocation end;
};

struct CompilerContext;
struct IRInstr;
struct ASTNode;
enum class IROp;

std::string token_to_string(const Token &token, bool verbose = false);
std::string IRInstr_to_string(const IRInstr &instr);
std::string_view to_string(TokenType type);
std::string_view to_string(IROp op);

void printAST(const std::shared_ptr<ASTNode> &node, int depth = 0);

void printErrors(CompilerContext &ctx);
