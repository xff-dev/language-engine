#include "vm.hpp"
#include "../common/diagnostics.hpp"
#include "builtins.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

VirtualMachine::VirtualMachine(std::vector<IRInstr> instructions,
                               VMContext &vmctx)
    : instructions(instructions), vmctx(vmctx) {
  vmctx.frames.push_back({
      .returnIP = -1,
      .result = "",
  });
}

std::string &VirtualMachine::getVariable(std::string var) {
  return vmctx.frames.back().variableMap[var];
}

void VirtualMachine::pushFrame(int returnIP, std::string result) {
  vmctx.frames.push_back({
      .returnIP = returnIP,
      .result = result,
      .variableMap = vmctx.frames.back().variableMap,
  });
}

void VirtualMachine::popFrame() { vmctx.frames.pop_back(); }

void VirtualMachine::skipFunctionDefinitions(int &ip) {
  int depth = 1;

  while (depth > 0) {
    ip++;

    auto ins = instructions[ip];

    if (ins.instr == IROp::FuncBegin) {
      depth++;
    } else if (ins.instr == IROp::FuncEnd) {
      depth--;
    }
  }
}

VMResult VirtualMachine::run() {
  for (int i = 0; i < instructions.size(); i++) {
    auto instr = instructions[i];
    if (instr.instr == IROp::Label) {
      vmctx.labelMap[instr.value] = i;
    }
    if (instr.instr == IROp::FuncBegin) {
      vmctx.functionMap[instr.value] = {i, instr.args};
    }
  }
  try {

    while (ip < instructions.size()) {
      const auto &ins = instructions[ip];

      switch (ins.instr) {
      case IROp::Const:
        getVariable(ins.result) = ins.value;
        break;
      case IROp::Mov:
        getVariable(ins.result) = getVariable(ins.value);
        break;

      case IROp::Jmp:
        ip = vmctx.labelMap[ins.value];
        break;
      case IROp::JmpIf:
        if (builtins::isTruthy(getVariable(ins.args[0]))) {
          ip = vmctx.labelMap[ins.value];
        }
        break;
      case IROp::JmpIfNot:
        if (!builtins::isTruthy(getVariable(ins.args[0]))) {
          ip = vmctx.labelMap[ins.value];
        }
        break;
      case IROp::Call:
        if (builtins::isBuiltin(ins.value)) {
          getVariable(ins.result) =
              builtins::callBuiltin(ins.value, ins.args, vmctx);
        } else {
          pushFrame(ip, ins.result);
          auto func = vmctx.functionMap[ins.value];
          for (int argn = 0; argn < ins.args.size(); argn++) {
            getVariable(func.second[argn]) = getVariable(ins.args[argn]);
          }
          ip = func.first;
        }
        break;
      case IROp::FuncEnd:
      case IROp::Return: {
        Frame frame = vmctx.frames.back();
        ip = frame.returnIP;
        std::string result = "";
        if (ins.args.size() == 1) {
          result = getVariable(ins.args[0]);
        }
        popFrame();
        getVariable(frame.result) = result;
        break;
      }

      case IROp::FuncBegin:
        skipFunctionDefinitions(ip);
        break;
      case IROp::Label:
        break;
        // default:
        //   // std::cout << to_string(instr.instr) << std::endl;
        //   break;
      }

      ip++;

      if (!vmctx.quiet) {
        std::cout << vmctx.stdoutBuffer.substr(vmctx.stdoutBufferPointer);

        vmctx.stdoutBufferPointer = vmctx.stdoutBuffer.size();
      }
    }
  } catch (std::runtime_error error) {
    return {
        .ok = false,
        .errorMessage = error.what(),
        .errorIP = -1,
        .stdoutBuffer = vmctx.stdoutBuffer,
    };
  }
  // for (auto [i, j] : variableMap) {
  //   std::cout << i << " " << j << std::endl;
  // }
  // for (auto [i, j] : labelMap) {
  //   std::cout << ":" << i << " " << j << std::endl;
  // }
  return {
      .ok = true,
      .stdoutBuffer = vmctx.stdoutBuffer,
  };
}
