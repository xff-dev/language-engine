#pragma once
#include "../common/context.hpp"
#include "token.hpp"
#include <cstddef>
#include <string>
#include <vector>

class Lexer {
public:
  explicit Lexer(std::string source, CompilerContext &ctx);

  std::vector<Token> tokenize();

private:
  const char current();
  void advance();

  Token parseInt();
  Token parseString();
  Token parseIdentifier();

private:
  std::string source;
  size_t pos = 0;
  int line = 1;
  int column = 1;
  CompilerContext &ctx;
};
