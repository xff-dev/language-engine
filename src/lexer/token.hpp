#pragma once
#include "string"

enum class TokenType {
  Identifier,
  Number,
  String,

  LParen,
  RParen,
  LBrace,
  RBrace,

  Comma,
  Semicolon,
  Equal,

  KeywordFunc,
  KeywordIf,
  KeywordReturn,
  KeywordWhile,

  EndOfFile
};

struct SourceLocation {
  int line;
  int column;
};

struct Token {
  TokenType type;
  std::string value;
  SourceLocation location;
};
