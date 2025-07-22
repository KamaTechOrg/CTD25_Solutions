#pragma once
#include <string>
#include <utility>
#include <variant>
#include <vector>

using Cell = std::pair<int, int>;

struct Command {
  long long timestamp_ms;
  std::string pieceId;
  std::string type;                              // "Move" | "Jump" | …
  using Param = std::variant<Cell, std::string>; // ← add
  std::vector<Param> params;
};
