// Piece.h
#pragma once
#include "Board.h"
#include "State.h"

class Piece {
public:
  Piece(std::string id, State *idle);

  void onCommand(const Command &cmd, long long now);
  void reset(long long startMs);
  void update(long long now);
  void drawOnBoard(Board &b, long long now);

  std::string id;
  State *state{nullptr};
  bool hasMoved{false};
};

