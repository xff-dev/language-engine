#pragma once

#include "vm.hpp"
#include <string>
#include <vector>

namespace builtins {

bool isTruthy(std::string value);
bool isBuiltin(std::string name);
std::string callBuiltin(std::string name, std::vector<std::string> args,
                        VMContext &vmctx);
bool checkArgLen(std::string name, int argn);
} // namespace builtins
