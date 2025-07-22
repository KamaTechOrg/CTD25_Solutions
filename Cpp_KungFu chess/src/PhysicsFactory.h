#pragma once
#include "Board.h"
#include "Physics.h"
#include <memory>

class PhysicsFactory {
public:
  explicit PhysicsFactory(Board &b) : board_(b) {}

  /** Create a Physics instance attached to `board_`.
      `cfg` is a JSON‑like object; only "speed_m_per_sec" is recognised. */
  template <typename Json>
  std::shared_ptr<Physics> create(std::pair<int, int> startCell,
                                  const Json &cfg) const {
    double speed = cfg.contains("speed_m_per_sec")
                       ? cfg["speed_m_per_sec"].get<double>()
                       : 1.0;
    return std::make_shared<Physics>(startCell, board_, speed);
  }

private:
  Board &board_;
};
