#include "builtins.hpp"
#include "functional"
#include "vm.hpp"
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

//     "print", "add", "sub", "mul", "div", "eq", "not",

namespace builtins {

using BuiltinFn = std::function<std::string(const std::vector<std::string> &,
                                            VMContext &vmctx)>;

std::optional<int> tryParseInt(const std::string &s) {
  try {
    size_t pos = 0;

    int value = std::stoi(s, &pos);

    if (pos != s.size()) {
      return std::nullopt;
    }

    return value;

  } catch (const std::invalid_argument &) {
    return std::nullopt;

  } catch (const std::out_of_range &) {
    return std::nullopt;
  }
}

void expectArgSize(std::string name, const std::vector<std::string> &args,
                   int n) {
  if (args.size() != n) {
    throw std::runtime_error(
        std::format("expected {} args in {}, got {}", n, name, args.size()));
  }
}

inline std::string getArg(std::string arg, VMContext &vmctx) {
  return vmctx.frames.back().variableMap[arg];
}

std::unordered_map<std::string, BuiltinFn> registry = {
    {"print",
     [](const std::vector<std::string> &args, VMContext &vmctx) -> std::string {
       for (int i = 0; i < args.size() - 1; i++) {
         auto arg = getArg(args[i], vmctx);
         vmctx.stdoutBuffer += arg + " ";
       }
       auto arg = args.back();
       vmctx.stdoutBuffer += getArg(arg, vmctx);
       vmctx.stdoutBuffer += "\n";
       return "";
     }},
    {
        "add",
        [](const std::vector<std::string> &args,
           VMContext &vmctx) -> std::string {
          expectArgSize("add", args, 2);

          auto a = tryParseInt(getArg(args[0], vmctx));
          auto b = tryParseInt(getArg(args[1], vmctx));
          if (a && b) {
            return std::to_string(*a + *b);
          }
          return getArg(args[0], vmctx) + getArg(args[1], vmctx);
        },
    },
    {
        "sub",
        [](const std::vector<std::string> &args,
           VMContext &vmctx) -> std::string {
          expectArgSize("sub", args, 2);

          auto a = tryParseInt(getArg(args[0], vmctx));
          auto b = tryParseInt(getArg(args[1], vmctx));
          if (a && b) {
            return std::to_string(*a - *b);
          }
          throw std::runtime_error("both numbers have to be integers");
        },
    },
    {
        "mul",
        [](const std::vector<std::string> &args,
           VMContext &vmctx) -> std::string {
          expectArgSize("sub", args, 2);

          auto a = tryParseInt(getArg(args[0], vmctx));
          auto b = tryParseInt(getArg(args[1], vmctx));
          if (a && b) {
            return std::to_string(*a * *b);
          }
          throw std::runtime_error("both numbers have to be integers");
        },
    },
    {
        "div",
        [](const std::vector<std::string> &args,
           VMContext &vmctx) -> std::string {
          expectArgSize("div", args, 2);

          auto a = tryParseInt(getArg(args[0], vmctx));
          auto b = tryParseInt(getArg(args[1], vmctx));
          if (a && b) {
            if (*b == 0) {
              throw std::runtime_error("division by zero");
            }
            return std::to_string(*a / *b);
          }

          throw std::runtime_error("both numbers have to be integers");
        },
    },
    {
        "not",
        [](const std::vector<std::string> &args,
           VMContext &vmctx) -> std::string {
          expectArgSize("not", args, 1);

          return isTruthy(getArg(args[0], vmctx)) ? "0" : "1";
        },
    },
    {
        "eq",
        [](const std::vector<std::string> &args,
           VMContext &vmctx) -> std::string {
          expectArgSize("eq", args, 2);

          return getArg(args[0], vmctx) == getArg(args[1], vmctx) ? "1" : "0";
        },
    },
};

bool isTruthy(std::string value) { return value != "0" && value != ""; }
bool isBuiltin(std::string name) {
  for (auto [k, v] : registry) {
    if (k == name) {
      return true;
    }
  }
  return false;
}

std::string callBuiltin(std::string name, std::vector<std::string> args,
                        VMContext &vmctx) {
  auto it = registry.find(name);

  return it->second(args, vmctx);
}

} // namespace builtins
