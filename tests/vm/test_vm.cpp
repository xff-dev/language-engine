#include "../../src/lexer/lexer.hpp"
#include "../../src/lowering/lowering.hpp"
#include "../../src/parser/parser.hpp"
#include "../../src/vm/vm.hpp"

#include "catch2/catch_test_macros.hpp"
#include <memory>
#include <string>

VMResult run_code(std::string source) {
  CompilerContext ctx;
  Lexer lexer(source, ctx);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, ctx);
  auto rootNode = parser.parse();

  IRGenerator generator(rootNode, ctx);
  auto generatorResult = generator.generate();

  VMContext vmctx{.ctx = ctx, .quiet = true};
  VirtualMachine vm(generatorResult, vmctx);

  return vm.run();
}

TEST_CASE("basic printing", "[vm][atomic]") {
  auto result = run_code("print(\"Hello, World!\");");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "Hello, World!\n");
}

TEST_CASE("basic printing with variable", "[vm][atomic]") {
  auto result = run_code("x = \"Hello, World!\"; print(x);");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "Hello, World!\n");
}

TEST_CASE("variable assignment", "[vm][atomic]") {
  auto result = run_code("x = 5; print(x);");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "5\n");
}

TEST_CASE("if statement(true)", "[vm][atomic]") {
  auto result = run_code("x = 5; if (x) { print(\"True\"); } ");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "True\n");
}

TEST_CASE("if statement(false)", "[vm][atomic]") {
  auto result = run_code("x = 0; if (x) { print(\"True\"); }");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "");
}

TEST_CASE("while loop", "[vm][atomic]") {
  auto result =
      run_code("x = 0; while (sub(x, 3)) { print(x); x = add(x, 1); }");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "0\n1\n2\n");
}

TEST_CASE("add function", "[vm][atomic][built-in]") {
  auto result = run_code("print(add(2, 3));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "5\n");
}

TEST_CASE("sub function", "[vm][atomic][built-in]") {
  auto result = run_code("print(sub(5, 2));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "3\n");
}

TEST_CASE("not function", "[vm][atomic][built-in]") {
  auto result = run_code("print(not(0)); print(not(1));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "1\n0\n");
}

TEST_CASE("print function", "[vm][atomic][built-in]") {
  auto result = run_code("print(\"Hello\");");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "Hello\n");
}

TEST_CASE("mul function", "[vm][atomic][built-in]") {
  auto result = run_code("print(mul(2, 3));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "6\n");
}

TEST_CASE("div function", "[vm][atomic][built-in]") {
  auto result = run_code("print(div(6, 3));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "2\n");
}

TEST_CASE("div by zero", "[vm][atomic][built-in]") {
  auto result = run_code("print(div(6, 0));");
  CHECK(!result.ok);
}

TEST_CASE("eq function", "[vm][atomic][built-in]") {
  auto result = run_code("print(eq(2, 2)); print(eq(2, 3));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "1\n0\n");
}

TEST_CASE("add with strings", "[vm][atomic][built-in]") {
  auto result = run_code("print(add(\"Hello, \", \"World!\"));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "Hello, World!\n");
}

TEST_CASE("sub with strings", "[vm][atomic][built-in]") {
  auto result = run_code("print(sub(\"Hello, World!\", \"World\"));");
  CHECK(!result.ok);
}

TEST_CASE("mul with strings", "[vm][atomic][built-in]") {
  auto result = run_code("print(mul(\"Hello\", 3));");
  CHECK(!result.ok);
}

TEST_CASE("div with strings", "[vm][atomic][built-in]") {
  auto result = run_code("print(div(\"Hello\", 3));");
  CHECK(!result.ok);
}

TEST_CASE("variable reassignment", "[vm][atomic]") {
  auto result = run_code("x = 5; print(x); x = \"Hello\"; print(x);");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "5\nHello\n");
}

TEST_CASE("simple function definition and call", "[vm][atomic]") {
  auto result = run_code("func greet() { print(\"Hello\"); } greet();");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "Hello\n");
}

TEST_CASE("function with parameters", "[vm][atomic]") {
  auto result =
      run_code("func addOne(x) { return add(x, 1); } print(addOne(5));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "6\n");
}

TEST_CASE("function with multiple parameters", "[vm][atomic]") {
  auto result =
      run_code("func add(x, y) { return add(x, y); } print(add(2, 3));");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "5\n");
}

TEST_CASE("recursive function", "[vm]") {
  auto result = run_code(R"(
    print(factorial(5));
    func factorial(n) {
      if (eq(n, 0)) {
        return 1;
      }         
      return mul(n, factorial(sub(n, 1)));
    }
  )");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "120\n");
}

TEST_CASE("recursive function 2", "[vm]") {
  auto result = run_code(R"(
    func fib(n) {
      if (eq(n, 0)) {
        return 0;
      }
      if (eq(n, 1)) {
        return 1;
      }
      return add(fib(sub(n, 1)), fib(sub(n, 2)));
    }
    print(fib(10));
  )");
  CHECK(result.ok);
  CHECK(result.stdoutBuffer == "55\n");
}
