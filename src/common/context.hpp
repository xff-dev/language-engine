#pragma once
#include "diagnostics.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct CompilerContext {
  std::vector<CompilerError> errors;
  std::string filename;
  std::string sourceCode;
  std::vector<std::unordered_map<std::string, std::string>> variableNamespaces;
};
