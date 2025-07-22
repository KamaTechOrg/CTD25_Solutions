// Piece.cpp
#include "Piece.h"

Piece::Piece(std::string i, State *idle) : id(std::move(i)), state(idle) {}

void Piece::onCommand(const Command &cmd, long long now) {
  state = state->getNext(cmd, now);
}

void Piece::reset(long long startMs) {
  state->reset(Command{startMs, id, "Idle", {}});
}

void Piece::update(long long now) { state = state->update(now); }

void Piece::drawOnBoard(Board &b, long long now) {
  auto [x, y] = state->physics->getPosPx();
  state->graphics.getImg().drawOn(b.img, x, y);

  long long remain = state->cooldownEndMs - now;
  if (remain > 0) {
    double secs = remain / 1000.0;
    cv::putText(b.img.mat(), std::to_string(secs), {x + 4, y + 16},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, {0, 0, 255, 255}, 1,
                cv::LINE_AA);
  }
}
