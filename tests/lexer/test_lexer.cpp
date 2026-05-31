#include "../../src/lexer/lexer.hpp"
#include "catch2/catch_test_macros.hpp"

#include <catch2/catch_all.hpp>
#include <vector>

std::pair<std::vector<Token>, CompilerContext> lex(std::string code) {
  CompilerContext ctx;
  Lexer lexer(code, ctx);
  return {lexer.tokenize(), ctx};
}

TEST_CASE("string", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("\"test\"");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::String);
  CHECK(tokens[0].value == "test");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("escaped string", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("\"\\n \\\\ \\\"\"");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::String);
  CHECK(tokens[0].value == "\n \\ \"");
}

TEST_CASE("int", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("123456");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::Number);
  CHECK(tokens[0].value == "123456");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("ident", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("test");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::Identifier);
  CHECK(tokens[0].value == "test");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("ident with underscore", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("test_ident");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::Identifier);
  CHECK(tokens[0].value == "test_ident");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("ident with digits", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("test123");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::Identifier);
  CHECK(tokens[0].value == "test123");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("single-character tokens", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("(){},;");

  REQUIRE(tokens.size() == 7);
  CHECK(tokens[0].type == TokenType::LParen);
  CHECK(tokens[0].value == "(");
  CHECK(tokens[1].type == TokenType::RParen);
  CHECK(tokens[1].value == ")");
  CHECK(tokens[2].type == TokenType::LBrace);
  CHECK(tokens[2].value == "{");
  CHECK(tokens[3].type == TokenType::RBrace);
  CHECK(tokens[3].value == "}");
  CHECK(tokens[4].type == TokenType::Comma);
  CHECK(tokens[4].value == ",");
  CHECK(tokens[5].type == TokenType::Semicolon);
  CHECK(tokens[5].value == ";");
  CHECK(tokens[6].type == TokenType::EndOfFile);
}

TEST_CASE("keyword func", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("func");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::KeywordFunc);
  CHECK(tokens[0].value == "func");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("keyword if", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("if");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::KeywordIf);
  CHECK(tokens[0].value == "if");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("keyword return", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("return");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::KeywordReturn);
  CHECK(tokens[0].value == "return");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("ident similar to keyword", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("return_value");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::Identifier);
  CHECK(tokens[0].value == "return_value");
  CHECK(tokens[1].type == TokenType::EndOfFile);
}

TEST_CASE("end of file", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("");

  REQUIRE(tokens.size() == 1);
  CHECK(tokens[0].type == TokenType::EndOfFile);
  CHECK(tokens[0].value == "");
}

TEST_CASE("simple function", "[lexer]") {
  auto [tokens, ctx] = lex("func add(a, b) { "
                           "  set(x, add(a, b)); "
                           "  return x;      "
                           "}");

  REQUIRE(tokens.size() == 25);
  CHECK(ctx.errors.size() == 0);

  CHECK(tokens[0].type == TokenType::KeywordFunc);
  CHECK(tokens[0].value == "func");
  CHECK(tokens[1].type == TokenType::Identifier);
  CHECK(tokens[1].value == "add");
  CHECK(tokens[2].type == TokenType::LParen);
  CHECK(tokens[3].type == TokenType::Identifier);
  CHECK(tokens[3].value == "a");
  CHECK(tokens[4].type == TokenType::Comma);
  CHECK(tokens[5].type == TokenType::Identifier);
  CHECK(tokens[5].value == "b");
  CHECK(tokens[6].type == TokenType::RParen);
  CHECK(tokens[7].type == TokenType::LBrace);
  CHECK(tokens[8].type == TokenType::Identifier);
  CHECK(tokens[8].value == "set");
  CHECK(tokens[9].type == TokenType::LParen);
  CHECK(tokens[10].type == TokenType::Identifier);
  CHECK(tokens[10].value == "x");
  CHECK(tokens[11].type == TokenType::Comma);
  CHECK(tokens[12].type == TokenType::Identifier);
  CHECK(tokens[12].value == "add");
  CHECK(tokens[13].type == TokenType::LParen);
  CHECK(tokens[14].type == TokenType::Identifier);
  CHECK(tokens[14].value == "a");
  CHECK(tokens[15].type == TokenType::Comma);
  CHECK(tokens[16].type == TokenType::Identifier);
  CHECK(tokens[16].value == "b");
  CHECK(tokens[17].type == TokenType::RParen);
  CHECK(tokens[18].type == TokenType::RParen);
  CHECK(tokens[19].type == TokenType::Semicolon);
  CHECK(tokens[20].type == TokenType::KeywordReturn);
  CHECK(tokens[21].type == TokenType::Identifier);
  CHECK(tokens[21].value == "x");
  CHECK(tokens[22].type == TokenType::Semicolon);
  CHECK(tokens[23].type == TokenType::RBrace);
  CHECK(tokens[24].type == TokenType::EndOfFile);
}

TEST_CASE("unexpected token", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("@");

  REQUIRE(tokens.size() == 1);
  CHECK(tokens[0].type == TokenType::EndOfFile);
  CHECK(ctx.errors.size() == 1);
  CHECK(ctx.errors[0].type == CompilerErrorType::UnexpectedToken);
  CHECK(ctx.errors[0].start.line == 1);
  CHECK(ctx.errors[0].start.column == 1);
}

TEST_CASE("unterminated string", "[lexer][atomic]") {
  auto [tokens, ctx] = lex("\"test");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].type == TokenType::String);
  CHECK(tokens[0].value == "test");
  CHECK(tokens[1].type == TokenType::EndOfFile);
  CHECK(ctx.errors.size() == 1);
  CHECK(ctx.errors[0].type == CompilerErrorType::UnterminatedString);
}
