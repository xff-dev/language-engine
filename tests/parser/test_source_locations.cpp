#include "../../src/lexer/lexer.hpp"
#include "../../src/parser/parser.hpp"

#include "catch2/catch_test_macros.hpp"
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {

std::shared_ptr<ASTNode> parse_code_for_locations(const std::string &source) {
  CompilerContext ctx;
  Lexer lexer(source, ctx);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, ctx);
  return parser.parse();
}

std::vector<std::string> split_source_lines(const std::string &source) {
  std::vector<std::string> lines{""};

  for (char ch : source) {
    if (ch == '\n') {
      lines.push_back("");
    } else {
      lines.back() += ch;
    }
  }

  return lines;
}

void require_source_location_in_source(const ASTNode &node,
                                       const std::string &nodeName,
                                       const std::vector<std::string> &lines) {
  INFO(nodeName);
  CHECK(node.location.line >= 1);
  CHECK(node.location.line <= static_cast<int>(lines.size()));

  if (node.location.line >= 1 &&
      node.location.line <= static_cast<int>(lines.size())) {
    const auto &line = lines[node.location.line - 1];
    CHECK(node.location.column >= 0);
    CHECK(node.location.column <= static_cast<int>(line.size()) + 1);
  }
}

void require_all_source_locations(const std::shared_ptr<ASTNode> &node,
                                  const std::vector<std::string> &lines,
                                  std::set<std::string> &visitedTypes) {
  REQUIRE(node != nullptr);

  if (auto typed = std::dynamic_pointer_cast<NumberNode>(node)) {
    visitedTypes.insert("NumberNode");
    require_source_location_in_source(*typed, "NumberNode", lines);
  } else if (auto typed = std::dynamic_pointer_cast<StringNode>(node)) {
    visitedTypes.insert("StringNode");
    require_source_location_in_source(*typed, "StringNode", lines);
  } else if (auto typed = std::dynamic_pointer_cast<VariableNode>(node)) {
    visitedTypes.insert("VariableNode");
    require_source_location_in_source(*typed, "VariableNode", lines);
  } else if (auto typed = std::dynamic_pointer_cast<CallNode>(node)) {
    visitedTypes.insert("CallNode");
    require_source_location_in_source(*typed, "CallNode", lines);
    for (const auto &arg : typed->args) {
      require_all_source_locations(arg, lines, visitedTypes);
    }
  } else if (auto typed = std::dynamic_pointer_cast<BlockNode>(node)) {
    visitedTypes.insert("BlockNode");
    require_source_location_in_source(*typed, "BlockNode", lines);
    for (const auto &statement : typed->statements) {
      require_all_source_locations(statement, lines, visitedTypes);
    }
  } else if (auto typed = std::dynamic_pointer_cast<IfNode>(node)) {
    visitedTypes.insert("IfNode");
    require_source_location_in_source(*typed, "IfNode", lines);
    require_all_source_locations(typed->condition, lines, visitedTypes);
    require_all_source_locations(typed->body, lines, visitedTypes);
  } else if (auto typed = std::dynamic_pointer_cast<WhileNode>(node)) {
    visitedTypes.insert("WhileNode");
    require_source_location_in_source(*typed, "WhileNode", lines);
    require_all_source_locations(typed->condition, lines, visitedTypes);
    require_all_source_locations(typed->body, lines, visitedTypes);
  } else if (auto typed = std::dynamic_pointer_cast<FunctionNode>(node)) {
    visitedTypes.insert("FunctionNode");
    require_source_location_in_source(*typed, "FunctionNode", lines);
    require_all_source_locations(typed->body, lines, visitedTypes);
  } else if (auto typed = std::dynamic_pointer_cast<ReturnNode>(node)) {
    visitedTypes.insert("ReturnNode");
    require_source_location_in_source(*typed, "ReturnNode", lines);
    if (typed->value != nullptr) {
      require_all_source_locations(typed->value, lines, visitedTypes);
    }
  } else if (auto typed = std::dynamic_pointer_cast<EqualNode>(node)) {
    visitedTypes.insert("EqualNode");
    require_source_location_in_source(*typed, "EqualNode", lines);
    require_all_source_locations(typed->value, lines, visitedTypes);
  } else {
    FAIL("Unknown AST node type in source location traversal");
  }
}

} // namespace

TEST_CASE("all AST nodes have source locations in complex program",
          "[parser][source-location]") {
  const std::string source = R"(func main(a, b) {
  total = sum(1, add(a, b), "done");
  if (total) {
    return total;
  }
  while (a) {
    tick();
  }
  return;
}
)";

  auto ast = parse_code_for_locations(source);
  auto lines = split_source_lines(source);
  std::set<std::string> visitedTypes;

  require_all_source_locations(ast, lines, visitedTypes);

  CHECK(visitedTypes.contains("BlockNode"));
  CHECK(visitedTypes.contains("CallNode"));
  CHECK(visitedTypes.contains("EqualNode"));
  CHECK(visitedTypes.contains("FunctionNode"));
  CHECK(visitedTypes.contains("IfNode"));
  CHECK(visitedTypes.contains("NumberNode"));
  CHECK(visitedTypes.contains("ReturnNode"));
  CHECK(visitedTypes.contains("StringNode"));
  CHECK(visitedTypes.contains("VariableNode"));
  CHECK(visitedTypes.contains("WhileNode"));
}

TEST_CASE("empty program root block has source location",
          "[parser][source-location]") {
  const std::string source = "";

  auto ast = parse_code_for_locations(source);
  auto lines = split_source_lines(source);
  std::set<std::string> visitedTypes;

  require_all_source_locations(ast, lines, visitedTypes);

  CHECK(visitedTypes == std::set<std::string>{"BlockNode"});
}
