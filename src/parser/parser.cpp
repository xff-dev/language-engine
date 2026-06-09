#include "parser.hpp"
#include "ASTnodes.hpp"
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

Parser::Parser(std::vector<Token> tokens, CompilerContext &ctx)
    : tokens(tokens), ctx(ctx) {};

Token &Parser::current() { return tokens[pos]; }
Token &Parser::peek(int idx) {
  if (pos + idx >= tokens.size()) {
    throw std::runtime_error("peek at index >= tokens.size()");
  }
  return tokens[pos + idx];
}

bool Parser::match(TokenType type) { return current().type == type; }

Token Parser::consume(TokenType type) {
  if (!match(type)) {
    std::string message = std::format("Expected {}, got {}", to_string(type),
                                      token_to_string(current()));

    CompilerError error{
        .type = CompilerErrorType::UnexpectedToken,
        .message = message,
        .start = current().location,
        .end = current().location,
    };
    ctx.errors.push_back(error);
    throw std::runtime_error(message);
  }
  Token &token = current();
  advance();
  return token;
}
void Parser::advance() { pos++; }

std::shared_ptr<ASTNode> Parser::parse() { return parseBlock(); }

std::shared_ptr<ASTNode> Parser::parseBlock() {
  std::vector<std::shared_ptr<ASTNode>> statements;
  auto location = current().location;

  while (!match(TokenType::EndOfFile) && !match(TokenType::RBrace)) {
    statements.push_back(parseStmt());
  }

  std::shared_ptr<BlockNode> node = std::make_shared<BlockNode>();
  node->statements = statements;
  node->location = location;
  return node;
}

std::shared_ptr<ASTNode> Parser::parseStmt() {
  if (match(TokenType::KeywordIf)) {
    return parseIf();
  } else if (match(TokenType::KeywordWhile)) {
    return parseWhile();
  } else if (match(TokenType::KeywordFunc)) {
    return parseFunction();
  } else if (match(TokenType::KeywordReturn)) {
    auto node = parseReturn();
    consume(TokenType::Semicolon);
    return node;
  } else {
    auto node = parseExpr();
    consume(TokenType::Semicolon);
    return node;
  }
}

std::shared_ptr<ASTNode> Parser::parseExpr() {
  if (match(TokenType::Identifier)) {
    if (peek().type == TokenType::LParen) {
      return parseCall();
    } else if (peek().type == TokenType::Equal) {
      return parseEqual();
    } else {
      return parseVariable();
    }
  } else if (match(TokenType::String)) {
    std::shared_ptr<StringNode> node = std::make_shared<StringNode>();
    node->value = consume(TokenType::String).value;
    node->location = current().location;
    return node;
  } else if (match(TokenType::Number)) {
    std::shared_ptr<NumberNode> node = std::make_shared<NumberNode>();
    node->value = consume(TokenType::Number).value;
    node->location = current().location;
    return node;
  }

  std::string message =
      std::format("Unexpected token {}", token_to_string(current()));

  CompilerError error{
      .type = CompilerErrorType::UnexpectedToken,
      .message = message,
      .start = current().location,
      .end = current().location,
  };
  ctx.errors.push_back(error);
  throw std::runtime_error(message);

  return {};
}

std::shared_ptr<ASTNode> Parser::parseVariable() {
  std::shared_ptr<VariableNode> node = std::make_shared<VariableNode>();

  node->location = current().location;
  node->value = consume(TokenType::Identifier).value;

  return node;
}

std::shared_ptr<ASTNode> Parser::parseCall() {
  std::shared_ptr<CallNode> node = std::make_shared<CallNode>();

  node->location = current().location;
  node->call = consume(TokenType::Identifier).value;
  consume(TokenType::LParen);

  while (current().type != TokenType::RParen) {
    node->args.push_back(parseExpr());
    if (!match(TokenType::RParen)) {
      consume(TokenType::Comma);
    }
  }
  consume(TokenType::RParen);

  return node;
}

std::shared_ptr<ASTNode> Parser::parseEqual() {
  std::shared_ptr<EqualNode> node = std::make_shared<EqualNode>();

  node->location = current().location;
  node->variable = consume(TokenType::Identifier).value;
  consume(TokenType::Equal);

  node->value = parseExpr();

  return node;
}

std::shared_ptr<ASTNode> Parser::parseFunction() {
  std::shared_ptr<FunctionNode> node = std::make_shared<FunctionNode>();

  node->location = current().location;
  consume(TokenType::KeywordFunc);
  node->name = consume(TokenType::Identifier).value;
  consume(TokenType::LParen);
  while (!match(TokenType::RParen)) {

    node->params.push_back(consume(TokenType::Identifier).value);
    if (!match(TokenType::RParen)) {
      consume(TokenType::Comma);
    }
  }
  consume(TokenType::RParen);
  consume(TokenType::LBrace);

  node->body = parseBlock();

  consume(TokenType::RBrace);

  return node;
}

std::shared_ptr<ASTNode> Parser::parseReturn() {
  std::shared_ptr<ReturnNode> node = std::make_shared<ReturnNode>();

  node->location = current().location;
  consume(TokenType::KeywordReturn);
  if (!match(TokenType::Semicolon)) {
    node->value = parseExpr();
  } else {
    node->value = nullptr;
  }
  return node;
}

std::shared_ptr<ASTNode> Parser::parseIf() {
  std::shared_ptr<IfNode> node = std::make_shared<IfNode>();

  node->location = current().location;
  consume(TokenType::KeywordIf);
  consume(TokenType::LParen);
  node->condition = parseExpr();
  consume(TokenType::RParen);
  consume(TokenType::LBrace);

  node->body = parseBlock();

  consume(TokenType::RBrace);

  return node;
}

std::shared_ptr<ASTNode> Parser::parseWhile() {
  std::shared_ptr<WhileNode> node = std::make_shared<WhileNode>();

  node->location = current().location;
  consume(TokenType::KeywordWhile);
  consume(TokenType::LParen);
  node->condition = parseExpr();
  consume(TokenType::RParen);
  consume(TokenType::LBrace);

  node->body = parseBlock();

  consume(TokenType::RBrace);

  return node;
}
