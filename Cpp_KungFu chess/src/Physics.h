#pragma once
#include "Board.h"
#include "Command.h"
#include <cmath>
#include <optional>

class Physics {
public:
  Physics(std::pair<int, int> startCell, const Board &b, double speedMps = 1.0);

  void reset(const Command &cmd);                 // mirrors Python reset()
  std::optional<Command> update(long long nowMs); // returns Arrived cmd
  std::pair<int, int> getPosPx() const;           // pixel coords

  bool canBeCaptured() const { return true; }
  bool canCapture() const { return true; }

  // public so State and tests can read
  std::pair<int, int> startCell, endCell;
  long long startMs{0};
  double durationMs{0};
  std::string mode{"Idle"};

private:
  const Board &board_;
  bool waitOnly_{false};
  mutable std::pair<int, int> currPx_{0, 0};
};
