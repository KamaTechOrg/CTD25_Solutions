#pragma once
#include "Board.h"
#include "Command.h"
#include "Img.h"
#include "Piece.h"
#include <chrono>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class InvalidBoard : public std::logic_error {
public:
  explicit InvalidBoard(const std::string &what) : std::logic_error(what) {}
};

class Game {
public:
  Game(std::vector<Piece *> pieces, Board board); // may throw
  void run();                                     // ESC quits
  void processInput(const Command &cmd);          // called from main

private:
  long long gameTimeMs() const;
  Board cloneBoard() const;
  static void mouseCb(int event, int x, int y, int flags, void *self);

  bool pathIsClear(std::pair<int, int> a, std::pair<int, int> b) const;
  void resolveCollisions();
  void checkPromotions();
  void promote(Piece *pawn, const std::string &code);

  bool validate(const std::vector<Piece *> &pcs) const;
  bool isWin() const;
  void announceWin() const;

  void draw();
  bool show();

  struct PairHash {
    size_t operator()(const std::pair<int, int> &p) const noexcept {
      return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
  };

  /* data */
  std::vector<Piece *> pieces_;
  Board board_;
  std::chrono::steady_clock::time_point startTime_;

  std::queue<Command> inputQ_;
  std::mutex qMutex_;
  std::optional<std::string> selectedId_;

  std::unordered_map<std::pair<int, int>, Piece *, PairHash> pos_;
  std::unordered_map<std::string, Piece *> byId_;

  Board currentBoard_;
};
