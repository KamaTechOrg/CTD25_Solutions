#include "Physics.h"

static constexpr double SLIDE_CELLS_PER_SEC = 4.0;

Physics::Physics(std::pair<int, int> cell, const Board &b, double)
    : startCell(cell), endCell(cell), board_(b) {}

void Physics::reset(const Command &cmd) {
  endCell = cmd.params.empty() ? startCell
                               : std::get<std::pair<int, int>>(cmd.params[0]);
  mode = cmd.type;
  startMs = cmd.timestamp_ms;

  double dy = endCell.first - startCell.first;
  double dx = endCell.second - startCell.second;
  double distCells = std::hypot(dx, dy);

  waitOnly_ = (distCells == 0 && cmd.type != "Idle");
  durationMs = std::max(200.0, distCells / SLIDE_CELLS_PER_SEC * 1000.0);
}

std::optional<Command> Physics::update(long long nowMs) {
  if (startCell == endCell && !waitOnly_)
    return std::nullopt;

  double t = std::min(1.0, (nowMs - startMs) / durationMs);

  double arcY = 0;
  if (mode == "Jump") {
    constexpr double HEIGHT = 30.0;
    arcY = -4 * HEIGHT * (t - 0.5) * (t - 0.5) + HEIGHT;
  }

  double curRow = startCell.first + (endCell.first - startCell.first) * t;
  double curCol = startCell.second + (endCell.second - startCell.second) * t;

  currPx_ = {int(std::round(curCol * board_.cellWpx)),
             int(std::round(curRow * board_.cellHpx - arcY))};

  if (t >= 1.0) {
    startCell = endCell;
    waitOnly_ = false;
    return Command{nowMs, "?", "Arrived", {}};
  }
  return std::nullopt;
}

std::pair<int, int> Physics::getPosPx() const { return currPx_; }
