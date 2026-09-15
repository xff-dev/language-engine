#pragma once

#include <string_view>

namespace AppConstants {
inline constexpr std::string_view Version = "0.1.0";

inline constexpr std::string_view Help =
    R"(Usage:
    mylang [options] file

Options:
    -h, --help
    -v, --version
    --tokens
    --ast
    ...
)";
} // namespace AppConstants
