#pragma once
#include "../common/diagnostics.hpp"
#include <string>
#include <vector>

enum class IROp {
  Const, // t = const value
  Mov,   // t = x

  Call,

  Label,
  Jmp,
  JmpIf,    // t != 0
  JmpIfNot, // t == 0

  FuncBegin,
  FuncEnd,
  Return,
};

struct IRInstr {
  IROp instr;

  std::string result;
  std::string value;

  std::vector<std::string> args;
};
