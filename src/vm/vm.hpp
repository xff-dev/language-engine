#pragma once

#include "../common/context.hpp"
#include "../lowering/ir.hpp"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct VMResult {
  bool ok;
  std::string errorMessage;
  int errorIP;
  std::string stdoutBuffer;
};

struct Frame {
  int returnIP;
  std::string result;
  std::unordered_map<std::string, std::string> variableMap;
};

struct VMContext {
  CompilerContext &ctx;
  std::unordered_map<std::string, int> labelMap;
  std::unordered_map<std::string, std::pair<int, std::vector<std::string>>>
      functionMap;
  std::vector<Frame> frames;
  std::string stdoutBuffer;
  int stdoutBufferPointer = 0;
  bool quiet = false;
};

class VirtualMachine {
public:
  VirtualMachine(std::vector<IRInstr> instructions, VMContext &vmctx);
  VMResult run();

private:
  int ip = 0;
  std::vector<IRInstr> instructions;
  VMContext &vmctx;
  std::string &getVariable(std::string var);
  void pushFrame(int returnIP, std::string result);
  void popFrame();
  void skipFunctionDefinitions(int &ip);
};
