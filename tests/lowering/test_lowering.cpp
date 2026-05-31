#include "../../src/lexer/lexer.hpp"
#include "../../src/lowering/lowering.hpp"
#include "../../src/parser/parser.hpp"
#include "catch2/catch_test_macros.hpp"

#include <catch2/catch_all.hpp>
#include <string>
#include <utility>

std::pair<std::vector<IRInstr>, CompilerContext>
lower_code(std::string source) {
  CompilerContext ctx;
  Lexer lexer(source, ctx);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, ctx);
  auto rootNode = parser.parse();

  IRGenerator generator(rootNode, ctx);

  return {generator.generate(), ctx};
}

std::vector<IRInstr> find(std::string value, IROp op,
                          std::vector<IRInstr> &hir) {
  std::vector<IRInstr> result;
  for (auto &instr : hir) {
    if (instr.instr == op && instr.value == value) {
      result.push_back(instr);
    }
  }
  return result;
}

std::vector<IRInstr> find(IROp op, std::vector<IRInstr> &hir) {
  std::vector<IRInstr> result;
  for (auto &instr : hir) {
    if (instr.instr == op) {
      result.push_back(instr);
    }
  }
  return result;
}

size_t findInstructionIndex(const std::vector<IRInstr> &hir, IROp op,
                            const std::string &value = "") {
  for (size_t i = 0; i < hir.size(); ++i) {
    if (hir[i].instr == op && (value.empty() || hir[i].value == value)) {
      return i;
    }
  }
  FAIL("instruction not found");
  return hir.size();
}

std::string resolveConst(const std::string &var, std::vector<IRInstr> &hir) {
  for (auto &instr : hir) {
    if (instr.result == var) {
      if (instr.instr == IROp::Const) {
        return instr.value;
      }

      if (instr.instr == IROp::Mov) {
        return resolveConst(instr.value, hir);
      }
    }
  }
  return "";
}

TEST_CASE("variable resolution", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = 1; y = x; z = y;");

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 3);

  auto resolvedZ = resolveConst(movs.back().value, hir);
  CHECK(resolvedZ == "1");
  CHECK(movs[1].value == movs[0].result);
  CHECK(movs[2].value == movs[1].result);
}

TEST_CASE("simple assignment", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = 1;");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 1);
  CHECK(movs[0].value == consts[0].result);
}

TEST_CASE("function call", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("print(1);");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  CHECK(calls[0].value == "print");

  CHECK(calls[0].args.size() == 1);
  CHECK(calls[0].args[0] == consts[0].result);
}

TEST_CASE("string literal", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = \"hello\";");

  auto consts = find("hello", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 1);

  CHECK(movs[0].value == consts[0].result);
}

TEST_CASE("variable assignment", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = 1; y = x;");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 2);

  auto resolved = resolveConst(movs.back().value, hir);
  CHECK(resolved == "1");
  CHECK(movs[1].value == movs[0].result);
}

TEST_CASE("variable reassignment", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = 1; x = 2;");

  auto consts1 = find("1", IROp::Const, hir);
  REQUIRE(consts1.size() == 1);

  auto consts2 = find("2", IROp::Const, hir);
  REQUIRE(consts2.size() == 1);

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 2);

  CHECK(movs[0].result == movs[1].result);

  auto resolved1 = resolveConst(movs[0].value, hir);
  CHECK(resolved1 == "1");

  auto resolved2 = resolveConst(movs[1].value, hir);
  CHECK(resolved2 == "2");
}

TEST_CASE("function result to variable assignment", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = print(1);");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  CHECK(calls[0].value == "print");

  CHECK(calls[0].args.size() == 1);
  CHECK(calls[0].args[0] == consts[0].result);

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 1);

  CHECK(movs.back().value == calls[0].result);
}

TEST_CASE("function call without arguments", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("foo();");

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  CHECK(calls[0].value == "foo");
  CHECK(calls[0].args.empty());
}

TEST_CASE("function call with mixed arguments", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("x = 3; foo(1, \"bar\", x);");

  auto const1 = find("1", IROp::Const, hir);
  REQUIRE(const1.size() == 1);

  auto constString = find("bar", IROp::Const, hir);
  REQUIRE(constString.size() == 1);

  auto movs = find(IROp::Mov, hir);
  REQUIRE(movs.size() == 1);

  auto calls = find("foo", IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  REQUIRE(calls[0].args.size() == 3);
  CHECK(calls[0].args[0] == const1[0].result);
  CHECK(calls[0].args[1] == constString[0].result);
  CHECK(calls[0].args[2] == movs[0].result);
}

TEST_CASE("nested function call", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("print(print(1));");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 2);

  CHECK(calls[0].value == "print");
  CHECK(calls[1].value == "print");

  REQUIRE(calls[0].args.size() == 1);
  REQUIRE(calls[1].args.size() == 1);

  CHECK(calls[0].args[0] == consts[0].result);
  CHECK(calls[1].args[0] == calls[0].result);
}

TEST_CASE("if statement", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("if (1) { print(2); }");

  auto consts1 = find("1", IROp::Const, hir);
  REQUIRE(consts1.size() == 1);

  auto consts2 = find("2", IROp::Const, hir);
  REQUIRE(consts2.size() == 1);

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  CHECK(calls[0].value == "print");

  CHECK(calls[0].args.size() == 1);
  CHECK(calls[0].args[0] == consts2[0].result);

  auto jmps = find(IROp::JmpIfNot, hir);
  REQUIRE(jmps.size() == 1);
  CHECK(jmps[0].args.size() == 1);
  CHECK(jmps[0].args[0] == consts1[0].result);

  auto labels = find(IROp::Label, hir);
  REQUIRE(labels.size() == 1);
  CHECK(jmps[0].value == labels[0].value);

  auto jmpIndex = findInstructionIndex(hir, IROp::JmpIfNot, jmps[0].value);
  auto callIndex = findInstructionIndex(hir, IROp::Call, "print");
  auto labelIndex = findInstructionIndex(hir, IROp::Label, labels[0].value);
  CHECK(jmpIndex < callIndex);
  CHECK(callIndex < labelIndex);
}

TEST_CASE("while loop", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("while (1) { print(2); }");

  auto consts1 = find("1", IROp::Const, hir);
  REQUIRE(consts1.size() == 1);

  auto consts2 = find("2", IROp::Const, hir);
  REQUIRE(consts2.size() == 1);

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  CHECK(calls[0].value == "print");

  CHECK(calls[0].args.size() == 1);
  CHECK(calls[0].args[0] == consts2[0].result);

  auto jmpsIfNot = find(IROp::JmpIfNot, hir);
  REQUIRE(jmpsIfNot.size() == 1);
  CHECK(jmpsIfNot[0].args.size() == 1);
  CHECK(jmpsIfNot[0].args[0] == consts1[0].result);

  auto jmps = find(IROp::Jmp, hir);
  REQUIRE(jmps.size() == 1);

  auto labels = find(IROp::Label, hir);
  REQUIRE(labels.size() == 2);

  CHECK(jmps[0].value == labels[0].value);
  CHECK(jmpsIfNot[0].value == labels[1].value);

  auto startLabelIndex =
      findInstructionIndex(hir, IROp::Label, labels[0].value);
  auto jmpIfNotIndex =
      findInstructionIndex(hir, IROp::JmpIfNot, labels[1].value);
  auto callIndex = findInstructionIndex(hir, IROp::Call, "print");
  auto backJmpIndex = findInstructionIndex(hir, IROp::Jmp, labels[0].value);
  auto exitLabelIndex = findInstructionIndex(hir, IROp::Label, labels[1].value);

  CHECK(startLabelIndex < jmpIfNotIndex);
  CHECK(jmpIfNotIndex < callIndex);
  CHECK(callIndex < backJmpIndex);
  CHECK(backJmpIndex < exitLabelIndex);
}

TEST_CASE("while loop lowers condition inside loop", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("while (keepGoing()) { print(1); }");

  auto labels = find(IROp::Label, hir);
  REQUIRE(labels.size() == 2);

  auto conditionCalls = find("keepGoing", IROp::Call, hir);
  REQUIRE(conditionCalls.size() == 1);

  auto jmpsIfNot = find(IROp::JmpIfNot, hir);
  REQUIRE(jmpsIfNot.size() == 1);
  CHECK(jmpsIfNot[0].args.size() == 1);
  CHECK(jmpsIfNot[0].args[0] == conditionCalls[0].result);

  auto startLabelIndex =
      findInstructionIndex(hir, IROp::Label, labels[0].value);
  auto conditionIndex = findInstructionIndex(hir, IROp::Call, "keepGoing");
  auto jmpIfNotIndex =
      findInstructionIndex(hir, IROp::JmpIfNot, labels[1].value);

  CHECK(startLabelIndex < conditionIndex);
  CHECK(conditionIndex < jmpIfNotIndex);
}

TEST_CASE("function definition", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("func foo() { print(1); }");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto calls = find(IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  CHECK(calls[0].value == "print");

  CHECK(calls[0].args.size() == 1);
  CHECK(calls[0].args[0] == consts[0].result);

  auto funcBegins = find(IROp::FuncBegin, hir);
  REQUIRE(funcBegins.size() == 1);
  CHECK(funcBegins[0].value == "foo");

  auto funcEnds = find(IROp::FuncEnd, hir);
  REQUIRE(funcEnds.size() == 1);

  auto funcBeginIndex = findInstructionIndex(hir, IROp::FuncBegin, "foo");
  auto callIndex = findInstructionIndex(hir, IROp::Call, "print");
  auto funcEndIndex = findInstructionIndex(hir, IROp::FuncEnd, "foo");

  CHECK(funcBeginIndex < callIndex);
  CHECK(callIndex < funcEndIndex);
}

TEST_CASE("function definition with parameters", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("func foo(a, b) { print(a); return b; }");

  auto funcBegins = find(IROp::FuncBegin, hir);
  REQUIRE(funcBegins.size() == 1);
  CHECK(funcBegins[0].value == "foo");
  REQUIRE(funcBegins[0].args.size() == 2);

  auto calls = find("print", IROp::Call, hir);
  REQUIRE(calls.size() == 1);
  REQUIRE(calls[0].args.size() == 1);
  CHECK(calls[0].args[0] == funcBegins[0].args[0]);

  auto returns = find(IROp::Return, hir);
  REQUIRE(returns.size() == 1);
  REQUIRE(returns[0].args.size() == 1);
  CHECK(returns[0].args[0] == funcBegins[0].args[1]);
}

TEST_CASE("function return", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("func foo() { return 1; }");

  auto consts = find("1", IROp::Const, hir);
  REQUIRE(consts.size() == 1);

  auto returns = find(IROp::Return, hir);
  REQUIRE(returns.size() == 1);
  CHECK(returns[0].args.size() == 1);
  CHECK(returns[0].args[0] == consts[0].result);

  auto funcBegins = find(IROp::FuncBegin, hir);
  REQUIRE(funcBegins.size() == 1);
  CHECK(funcBegins[0].value == "foo");

  auto funcEnds = find(IROp::FuncEnd, hir);
  REQUIRE(funcEnds.size() == 1);

  auto funcBeginIndex = findInstructionIndex(hir, IROp::FuncBegin, "foo");
  auto returnIndex = findInstructionIndex(hir, IROp::Return);
  auto funcEndIndex = findInstructionIndex(hir, IROp::FuncEnd, "foo");

  CHECK(funcBeginIndex < returnIndex);
  CHECK(returnIndex < funcEndIndex);
}

TEST_CASE("function return without value", "[lowering][atomic]") {
  auto [hir, ctx] = lower_code("func foo() { return; }");

  auto returns = find(IROp::Return, hir);
  REQUIRE(returns.size() == 1);
  CHECK(returns[0].args.empty());
}
