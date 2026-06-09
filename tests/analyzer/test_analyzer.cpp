#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "../../src/analyzer/analyzer.hpp"
#include "../../src/common/context.hpp"
#include "../../src/lexer/lexer.hpp"
#include "../../src/parser/parser.hpp"
#include "../../src/parser/ASTnodes.hpp"

// Helper to parse code and analyze it
std::shared_ptr<ASTNode> parseCode(const std::string &code,
                                    CompilerContext &ctx) {
  ctx.sourceCode = code;
  Lexer lexer(code, ctx);
  auto tokens = lexer.tokenize();
  Parser parser(tokens, ctx);
  return parser.parse();
}

TEST_CASE("Analyzer - Basic variable creation and usage",
           "[analyzer][atomic]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 5;", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Undefined variable usage", "[analyzer][atomic]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("print(x);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
  REQUIRE(ctx.errors[0].message.find("x") != std::string::npos);
}

TEST_CASE("Analyzer - Variable scope: local variable shadows outer",
           "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 1;\nprint(x);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function definition and call", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo() { }\nfoo();", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function redeclaration", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo() { }\nfunc foo() { }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::Redeclaration);
  REQUIRE(ctx.errors[0].message.find("redefinition") != std::string::npos);
}

TEST_CASE("Analyzer - Undefined function call", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("foo();", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedFunction);
  REQUIRE(ctx.errors[0].message.find("foo") != std::string::npos);
}

TEST_CASE("Analyzer - Function argument count mismatch",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo(a, b) { }\nfoo(1);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::FunctionArgumentMismatch);
  REQUIRE(ctx.errors[0].message.find("foo()") != std::string::npos);
}

TEST_CASE("Analyzer - Duplicate function parameters",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo(a, a) { }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::Redeclaration);
  REQUIRE(ctx.errors[0].message.find("parameter") != std::string::npos);
}

TEST_CASE("Analyzer - Function parameters are in scope inside function",
           "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo(x) { print(x); }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function parameters not in scope outside function",
           "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo(x) { }\nprint(x);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
}

TEST_CASE("Analyzer - Return outside function", "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("return 5;", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::ReturnOutsideFunction);
}

TEST_CASE("Analyzer - Return inside function is valid",
           "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo() { return 5; }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Variable assignment in function", "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo(x) { y = x;\nprint(y); }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - If statement condition is visited",
           "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("if (undefinedVar) { print(5); }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
  REQUIRE(ctx.errors[0].message.find("undefinedVar") != std::string::npos);
}

TEST_CASE("Analyzer - If statement body is visited",
           "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("if (1) { print(undefinedVar); }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
}

TEST_CASE("Analyzer - While statement condition is visited",
           "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("while (undefinedVar) { print(5); }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
}

TEST_CASE("Analyzer - While statement body is visited",
           "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("while (1) { print(undefinedVar); }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
}

TEST_CASE("Analyzer - Builtin function print with variable args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 5;\nprint(x);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Builtin function add with correct args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("add(1, 2);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Builtin function add with wrong arg count",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("add(1);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::FunctionArgumentMismatch);
}

TEST_CASE("Analyzer - Builtin function not with correct args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("not(1);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Builtin function eq with correct args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("eq(1, 2);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Builtin function sub with correct args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("sub(5, 3);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Builtin function mul with correct args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("mul(3, 4);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Builtin function div with correct args",
           "[analyzer][builtins]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("div(10, 2);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Multiple variables in sequence", "[analyzer][atomic]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 1;\ny = 2;\nz = 3;\nprint(x);\nprint(y);\nprint(z);",
                        ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Nested function calls", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 5;\ny = add(x, 3);\nprint(y);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function with return statement and no body statements",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo() { return 42; }\nfoo();", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Empty function body", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo() { }\nfoo();", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Return with no value", "[analyzer][control-flow]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo() { return; }", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Complex nested structure with variables in different "
           "scopes",
           "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 1;\nfunc foo(y) { z = add(x, y);\nreturn z; }\nprint(foo(5));",
                        ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Variable assignment in if statement",
           "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("if (1) { x = 5; }\nprint(x);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Variable assignment in while loop", "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 0;\nwhile (x) { y = 1; }\nprint(y);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Multiple function definitions", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode(
      "func foo() { return 1; }\nfunc bar() { return 2; }\nfunc baz() { return 3; "
      "}\nprint(foo());\nprint(bar());\nprint(baz());",
      ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function call with variable argument",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root =
      parseCode("func add_one(x) { return add(x, 1); }\ny = 5;\nprint(add_one(y));",
                ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Call with undefined variable in argument",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("foo(undefinedVar);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 2); // Undefined function and undefined variable
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedFunction);
  REQUIRE(ctx.errors[1].type == CompilerErrorType::UndefinedVariable);
}

TEST_CASE("Analyzer - Assignment with complex nested expression",
           "[analyzer][atomic]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 5;\ny = add(sub(x, 2), mul(3, 4));", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function defined inside if block",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("if (1) { func foo() { return 5; } }\nfoo();", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  // Functions in if blocks are correctly discovered by parseFunctions
  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function defined inside while block",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("while (0) { func bar() { return 10; } }\nbar();", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  // Functions in while blocks are correctly discovered by parseFunctions
  REQUIRE(ctx.errors.empty());
}
TEST_CASE("Analyzer - Nested if blocks with variables", "[analyzer][scope]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("if (1) { x = 5; if (1) { y = x; } }\nprint(y);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Multiple parameters in function", "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root =
      parseCode("func sum(a, b, c) { return add(add(a, b), c); }\nprint(sum(1, "
                "2, 3));",
                ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}

TEST_CASE("Analyzer - Function argument count too many",
           "[analyzer][function]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("func foo(a) { }\nfoo(1, 2);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  // Function argument mismatch is reported
  REQUIRE(ctx.errors.size() >= 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::FunctionArgumentMismatch);
}

TEST_CASE("Analyzer - Variable used before assignment is error",
           "[analyzer][atomic]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("print(undefined);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.size() == 1);
  REQUIRE(ctx.errors[0].type == CompilerErrorType::UndefinedVariable);
}

TEST_CASE("Analyzer - Variable reassignment allowed", "[analyzer][atomic]") {
  CompilerContext ctx;
  ctx.filename = "test.mylang";

  auto root = parseCode("x = 5;\nx = 10;\nprint(x);", ctx);

  Analyzer analyzer(ctx);
  analyzer.analyze(root);

  REQUIRE(ctx.errors.empty());
}