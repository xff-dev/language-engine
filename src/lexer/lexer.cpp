#include "lexer.hpp"
#include "token.hpp"
#include "unordered_map"
#include <cctype>
#include <string>
#include <vector>

std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::KeywordIf},
    {"func", TokenType::KeywordFunc},
    {"return", TokenType::KeywordReturn},
    {"while", TokenType::KeywordWhile}};

std::unordered_map<char, TokenType> charTokens = {
    {'(', TokenType::LParen}, {')', TokenType::RParen},
    {'{', TokenType::LBrace}, {'}', TokenType::RBrace},
    {',', TokenType::Comma},  {';', TokenType::Semicolon},
    {'=', TokenType::Equal},
};

Lexer::Lexer(std::string source, CompilerContext &ctx)
    : source(source), ctx(ctx) {}

const char Lexer::current() {
  if (pos >= source.size()) {
    return '\0';
  }
  return source[pos];
}

void Lexer::advance() {
  if (current() == '\n') {
    column = 0;
    line++;
  } else {
    column++;
  }

  pos++;
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> result;

  while (current() != '\0') {
    if (isspace(current())) {
      advance();
      continue;
    }

    if (isalpha(current())) {
      result.push_back(parseIdentifier());
      continue;
    }

    if (isdigit(current())) {
      result.push_back(parseInt());
      continue;
    }

    if (current() == '"') {
      result.push_back(parseString());
      continue;
    }

    if (charTokens.find(current()) != charTokens.end()) {
      Token token{.location = {.line = line, .column = column}};
      token.value = current();
      token.type = charTokens[current()];
      advance();

      result.push_back(token);
      continue;
    }

    CompilerError error{
        .type = CompilerErrorType::UnexpectedToken,
        .message = "Unexpected token",
        .start = {line, column},
        .end = {line, column},
    };

    ctx.errors.push_back(error);

    advance();
  }

  Token token{.location = {.line = line, .column = column}};
  token.type = TokenType::EndOfFile;
  token.value = "";

  result.push_back(token);

  return result;
}

Token Lexer::parseIdentifier() {
  Token token{.type = TokenType::Identifier,
              .location = {.line = line, .column = column}};

  while (isalnum(current()) || current() == '_') {
    token.value += current();
    advance();
  }

  if (keywords.find(token.value) != keywords.end()) {
    token.type = keywords[token.value];
  }

  return token;
}

Token Lexer::parseInt() {
  Token token{.type = TokenType::Number,
              .location = {.line = line, .column = column}};

  while (isdigit(current())) {
    token.value += current();
    advance();
  }

  return token;
}

Token Lexer::parseString() {
  Token token{.type = TokenType::String,
              .location = {.line = line, .column = column}};

  advance();

  while (current() != '"' && current() != '\0') {
    if (current() == '\\') {
      advance();
      if (current() == 'n') {
        token.value += '\n';
      } else {
        token.value += current();
      }
      advance();
      continue;
    }

    token.value += current();
    advance();
  }

  if (current() == '\0') {
    CompilerError error{
        .type = CompilerErrorType::UnterminatedString,
        .message = "Unterminated string",
        .start = token.location,
        .end = {line, column},
    };
    ctx.errors.push_back(error);
    return token;
  }

  advance();

  return token;
}
