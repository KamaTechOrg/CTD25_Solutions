#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using MoveRule = std::tuple<int, int, int>; // dr, dc, tag

class Moves {
public:
  explicit Moves(std::vector<MoveRule> rules, int boardW, int boardH)
      : rules_(std::move(rules)), W_(boardW), H_(boardH) {}

  // Construct from moves.txt on disk
  explicit Moves(const std::string &path, int boardW, int boardH);
  explicit Moves(const std::filesystem::path &p, int W, int H)
      : Moves(p.string(), W, H) {}



  std::vector<std::pair<int, int>> getMoves(int r, int c) const;
  bool legalFor(std::pair<int, int> src, std::pair<int, int> dst,
                bool firstMove, bool capture) const;

private:
  static MoveRule parseLine(const std::string &l);

  std::vector<MoveRule> rules_;
  int W_{8}, H_{8};
};
