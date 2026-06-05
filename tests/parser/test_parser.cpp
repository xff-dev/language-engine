#include "../../src/lexer/lexer.hpp"
#include "../../src/parser/parser.hpp"

#include "catch2/catch_test_macros.hpp"
#include <memory>
#include <string>

std::shared_ptr<ASTNode> parse_code(std::string source) {
  CompilerContext ctx;
  Lexer lexer(source, ctx);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, ctx);
  return parser.parse();
}

template <typename ExpectedType>
std::shared_ptr<ExpectedType>
assert_node_type(const std::shared_ptr<ASTNode> &node) {
  auto casted = std::dynamic_pointer_cast<ExpectedType>(node);
  REQUIRE(casted != nullptr);
  return casted;
}

TEST_CASE("simple function call", "[parser][atomic]") {
  auto ast = parse_code("add();");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto call = assert_node_type<CallNode>(block->statements[0]);
  CHECK(call->call == "add");
  REQUIRE(call->args.size() == 0);
}

TEST_CASE("function call with number argument", "[parser][atomic]") {
  auto ast = parse_code("add(123);");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto call = assert_node_type<CallNode>(block->statements[0]);
  CHECK(call->call == "add");
  REQUIRE(call->args.size() == 1);

  auto arg = assert_node_type<NumberNode>(call->args[0]);
  CHECK(arg->value == "123");
}

TEST_CASE("function call with string argument", "[parser][atomic]") {
  auto ast = parse_code("add(\"test\");");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto call = assert_node_type<CallNode>(block->statements[0]);
  CHECK(call->call == "add");
  REQUIRE(call->args.size() == 1);

  auto arg = assert_node_type<StringNode>(call->args[0]);
  CHECK(arg->value == "test");
}

TEST_CASE("function call with variable argument", "[parser][atomic]") {
  auto ast = parse_code("add(x);");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto call = assert_node_type<CallNode>(block->statements[0]);
  CHECK(call->call == "add");
  REQUIRE(call->args.size() == 1);

  auto arg = assert_node_type<VariableNode>(call->args[0]);
  CHECK(arg->value == "x");
}

TEST_CASE("function call with nested function call argument",
          "[parser][atomic]") {
  auto ast = parse_code("add(mul(2, 3));");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto call = assert_node_type<CallNode>(block->statements[0]);
  CHECK(call->call == "add");
  REQUIRE(call->args.size() == 1);

  auto nestedCall = assert_node_type<CallNode>(call->args[0]);
  CHECK(nestedCall->call == "mul");
  REQUIRE(nestedCall->args.size() == 2);

  auto arg1 = assert_node_type<NumberNode>(nestedCall->args[0]);
  CHECK(arg1->value == "2");

  auto arg2 = assert_node_type<NumberNode>(nestedCall->args[1]);
  CHECK(arg2->value == "3");
}

TEST_CASE("if statement", "[parser][atomic]") {
  auto ast = parse_code("if (x) { test(); }");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto ifNode = assert_node_type<IfNode>(block->statements[0]);
  auto condition = assert_node_type<VariableNode>(ifNode->condition);
  CHECK(condition->value == "x");

  auto body = assert_node_type<BlockNode>(ifNode->body);
  REQUIRE(body->statements.size() == 1);

  auto testNode = assert_node_type<CallNode>(body->statements[0]);
  CHECK(testNode->call == "test");
}

TEST_CASE("while statement", "[parser][atomic]") {
  auto ast = parse_code("while (x) { test(); }");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto whileNode = assert_node_type<WhileNode>(block->statements[0]);
  auto condition = assert_node_type<VariableNode>(whileNode->condition);
  CHECK(condition->value == "x");

  auto body = assert_node_type<BlockNode>(whileNode->body);
  REQUIRE(body->statements.size() == 1);

  auto testNode = assert_node_type<CallNode>(body->statements[0]);
  CHECK(testNode->call == "test");
}

TEST_CASE("function definition", "[parser][atomic]") {
  auto ast = parse_code("func add(a, b) { test(); }");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto funcNode = assert_node_type<FunctionNode>(block->statements[0]);
  CHECK(funcNode->name == "add");
  REQUIRE(funcNode->params.size() == 2);
  CHECK(funcNode->params[0] == "a");
  CHECK(funcNode->params[1] == "b");

  auto body = assert_node_type<BlockNode>(funcNode->body);
  REQUIRE(body->statements.size() == 1);

  auto testNode = assert_node_type<CallNode>(body->statements[0]);
  CHECK(testNode->call == "test");
}

TEST_CASE("return statement", "[parser][atomic]") {
  auto ast = parse_code("return x;");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto returnNode = assert_node_type<ReturnNode>(block->statements[0]);
  auto value = assert_node_type<VariableNode>(returnNode->value);
  CHECK(value->value == "x");
}

TEST_CASE("return statement without value", "[parser][atomic]") {
  auto ast = parse_code("return;");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto returnNode = assert_node_type<ReturnNode>(block->statements[0]);
  CHECK(returnNode->value == nullptr);
}

TEST_CASE("empty block", "[parser][atomic]") {
  auto ast = parse_code("");
  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 0);
}

TEST_CASE("multiple statements", "[parser][atomic]") {
  auto ast = parse_code("test(); \ntest2();");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 2);

  auto call1 = assert_node_type<CallNode>(block->statements[0]);
  CHECK(call1->call == "test");

  auto call2 = assert_node_type<CallNode>(block->statements[1]);
  CHECK(call2->call == "test2");
}

TEST_CASE("complex function definition", "[parser][complex]") {
  auto ast = parse_code(R"(
    func add(a, b) {
      set(x, add(a, b));
      if (x) {
        return x;
      }
      return 0;
    }
  )");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto funcNode = assert_node_type<FunctionNode>(block->statements[0]);
  CHECK(funcNode->name == "add");
  REQUIRE(funcNode->params.size() == 2);
  CHECK(funcNode->params[0] == "a");
  CHECK(funcNode->params[1] == "b");

  auto body = assert_node_type<BlockNode>(funcNode->body);
  REQUIRE(body->statements.size() == 3);

  auto setCall = assert_node_type<CallNode>(body->statements[0]);
  CHECK(setCall->call == "set");
  REQUIRE(setCall->args.size() == 2);
  auto xVar = assert_node_type<VariableNode>(setCall->args[0]);
  CHECK(xVar->value == "x");

  auto addCall = assert_node_type<CallNode>(setCall->args[1]);
  CHECK(addCall->call == "add");
  REQUIRE(addCall->args.size() == 2);
  auto addArg0 = assert_node_type<VariableNode>(addCall->args[0]);
  CHECK(addArg0->value == "a");
  auto addArg1 = assert_node_type<VariableNode>(addCall->args[1]);
  CHECK(addArg1->value == "b");

  auto ifNode = assert_node_type<IfNode>(body->statements[1]);
  auto condition = assert_node_type<VariableNode>(ifNode->condition);
  CHECK(condition->value == "x");

  auto ifBody = assert_node_type<BlockNode>(ifNode->body);
  REQUIRE(ifBody->statements.size() == 1);
  auto returnNode = assert_node_type<ReturnNode>(ifBody->statements[0]);
  auto returnVar = assert_node_type<VariableNode>(returnNode->value);
  CHECK(returnVar->value == "x");

  auto returnZero = assert_node_type<ReturnNode>(body->statements[2]);
  auto returnZeroValue = assert_node_type<NumberNode>(returnZero->value);
  CHECK(returnZeroValue->value == "0");
}

TEST_CASE("while loop with nested body", "[parser][complex]") {
  auto ast = parse_code(R"(
    while (cond) {
      compute(1, "x");
      test();
    }
  )");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto whileNode = assert_node_type<WhileNode>(block->statements[0]);
  auto condition = assert_node_type<VariableNode>(whileNode->condition);
  CHECK(condition->value == "cond");

  auto body = assert_node_type<BlockNode>(whileNode->body);
  REQUIRE(body->statements.size() == 2);

  auto computeCall = assert_node_type<CallNode>(body->statements[0]);
  CHECK(computeCall->call == "compute");
  REQUIRE(computeCall->args.size() == 2);
  auto computeArg0 = assert_node_type<NumberNode>(computeCall->args[0]);
  CHECK(computeArg0->value == "1");
  auto computeArg1 = assert_node_type<StringNode>(computeCall->args[1]);
  CHECK(computeArg1->value == "x");

  auto testCall = assert_node_type<CallNode>(body->statements[1]);
  CHECK(testCall->call == "test");
}

TEST_CASE("nested calls and mixed statements", "[parser][complex]") {
  auto ast = parse_code(R"(
    foo("a", add(1, mul(2, 3)), bar(x));
    return result;
  )");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 2);

  auto fooCall = assert_node_type<CallNode>(block->statements[0]);
  CHECK(fooCall->call == "foo");
  REQUIRE(fooCall->args.size() == 3);

  auto arg0 = assert_node_type<StringNode>(fooCall->args[0]);
  CHECK(arg0->value == "a");

  auto arg1 = assert_node_type<CallNode>(fooCall->args[1]);
  CHECK(arg1->call == "add");
  REQUIRE(arg1->args.size() == 2);
  auto addArg0 = assert_node_type<NumberNode>(arg1->args[0]);
  CHECK(addArg0->value == "1");

  auto mulCall = assert_node_type<CallNode>(arg1->args[1]);
  CHECK(mulCall->call == "mul");
  REQUIRE(mulCall->args.size() == 2);
  auto mulArg0 = assert_node_type<NumberNode>(mulCall->args[0]);
  CHECK(mulArg0->value == "2");
  auto mulArg1 = assert_node_type<NumberNode>(mulCall->args[1]);
  CHECK(mulArg1->value == "3");

  auto arg2 = assert_node_type<CallNode>(fooCall->args[2]);
  CHECK(arg2->call == "bar");
  REQUIRE(arg2->args.size() == 1);
  auto barArg0 = assert_node_type<VariableNode>(arg2->args[0]);
  CHECK(barArg0->value == "x");

  auto returnNode = assert_node_type<ReturnNode>(block->statements[1]);
  auto returnVar = assert_node_type<VariableNode>(returnNode->value);
  CHECK(returnVar->value == "result");
}

TEST_CASE("assignment with number", "[parser][atomic]") {
  auto ast = parse_code("x = 42;");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto equalNode = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equalNode->variable == "x");

  auto value = assert_node_type<NumberNode>(equalNode->value);
  CHECK(value->value == "42");
}

TEST_CASE("assignment with string", "[parser][atomic]") {
  auto ast = parse_code("msg = \"hello\";");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto equalNode = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equalNode->variable == "msg");

  auto value = assert_node_type<StringNode>(equalNode->value);
  CHECK(value->value == "hello");
}

TEST_CASE("assignment with variable", "[parser][atomic]") {
  auto ast = parse_code("result = input;");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto equalNode = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equalNode->variable == "result");

  auto value = assert_node_type<VariableNode>(equalNode->value);
  CHECK(value->value == "input");
}

TEST_CASE("assignment with function call", "[parser][atomic]") {
  auto ast = parse_code("total = sum(1, 2);");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto equalNode = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equalNode->variable == "total");

  auto callValue = assert_node_type<CallNode>(equalNode->value);
  CHECK(callValue->call == "sum");
  REQUIRE(callValue->args.size() == 2);

  auto arg0 = assert_node_type<NumberNode>(callValue->args[0]);
  CHECK(arg0->value == "1");
  auto arg1 = assert_node_type<NumberNode>(callValue->args[1]);
  CHECK(arg1->value == "2");
}

TEST_CASE("assignment with nested function call", "[parser][atomic]") {
  auto ast = parse_code("value = compute(add(5, 3));");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto equalNode = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equalNode->variable == "value");

  auto outerCall = assert_node_type<CallNode>(equalNode->value);
  CHECK(outerCall->call == "compute");
  REQUIRE(outerCall->args.size() == 1);

  auto innerCall = assert_node_type<CallNode>(outerCall->args[0]);
  CHECK(innerCall->call == "add");
  REQUIRE(innerCall->args.size() == 2);

  auto arg0 = assert_node_type<NumberNode>(innerCall->args[0]);
  CHECK(arg0->value == "5");
  auto arg1 = assert_node_type<NumberNode>(innerCall->args[1]);
  CHECK(arg1->value == "3");
}

TEST_CASE("multiple assignments", "[parser][atomic]") {
  auto ast = parse_code("a = 1;\nb = 2;");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 2);

  auto equal1 = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equal1->variable == "a");
  auto val1 = assert_node_type<NumberNode>(equal1->value);
  CHECK(val1->value == "1");

  auto equal2 = assert_node_type<EqualNode>(block->statements[1]);
  CHECK(equal2->variable == "b");
  auto val2 = assert_node_type<NumberNode>(equal2->value);
  CHECK(val2->value == "2");
}

TEST_CASE("assignment with multiple arguments to function",
          "[parser][atomic]") {
  auto ast = parse_code("result = process(\"a\", 100, var);");

  auto block = assert_node_type<BlockNode>(ast);
  REQUIRE(block->statements.size() == 1);

  auto equalNode = assert_node_type<EqualNode>(block->statements[0]);
  CHECK(equalNode->variable == "result");

  auto callValue = assert_node_type<CallNode>(equalNode->value);
  CHECK(callValue->call == "process");
  REQUIRE(callValue->args.size() == 3);

  auto arg0 = assert_node_type<StringNode>(callValue->args[0]);
  CHECK(arg0->value == "a");
  auto arg1 = assert_node_type<NumberNode>(callValue->args[1]);
  CHECK(arg1->value == "100");
  auto arg2 = assert_node_type<VariableNode>(callValue->args[2]);
  CHECK(arg2->value == "var");
}

// test node->sourceLocation is correct


