#pragma once
#include "../common/context.hpp"
#include "../lexer/token.hpp"

#include "ASTnodes.hpp"
#include <cstddef>
#include <memory>
#include <vector>

class Parser {
public:
  Parser(std::vector<Token> tokens, CompilerContext &ctx);

  std::shared_ptr<ASTNode> parse();

private:
  Token &current();
  Token &peek(int idx = 1);
  bool match(TokenType type);
  Token consume(TokenType type);
  void advance();

  std::shared_ptr<ASTNode> parseEqual();
  std::shared_ptr<ASTNode> parseBlock();
  std::shared_ptr<ASTNode> parseStmt();
  std::shared_ptr<ASTNode> parseExpr();
  std::shared_ptr<ASTNode> parseCall();
  std::shared_ptr<ASTNode> parseVariable();
  std::shared_ptr<ASTNode> parseIf();
  std::shared_ptr<ASTNode> parseFunction();
  std::shared_ptr<ASTNode> parseReturn();
  std::shared_ptr<ASTNode> parseWhile();

private:
  std::vector<Token> tokens;
  size_t pos = 0;
  CompilerContext &ctx;
};
