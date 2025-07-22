#include "Moves.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <sstream>

Moves::Moves(const std::string &path, int W, int H) : W_(W), H_(H) {
  std::ifstream f(path);
  if (!f)
    throw std::runtime_error("Cannot open moves file: " + path);
  std::string line;
  while (std::getline(f, line))
    if (auto t = parseLine(line);
        std::get<2>(t) != INT_MIN) // skip blanks/comments
      rules_.push_back(t);
}

MoveRule Moves::parseLine(const std::string &l) {
  auto noComment = l.substr(0, l.find('#'));
  auto s = noComment;
  s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
  if (s.empty())
    return {0, 0, INT_MIN};

  if (s.front() == '[' && s.back() == ']')
    s = s.substr(1, s.size() - 2);
  std::replace(s.begin(), s.end(), ',', ' ');
  std::stringstream ss(s);
  int dr{}, dc{}, tag{-1};
  ss >> dr >> dc;
  if (ss >> tag) {
  } // optionally read 3rd
  return {dr, dc, tag};
}

std::vector<std::pair<int, int>> Moves::getMoves(int r, int c) const {
  std::vector<std::pair<int, int>> out;
  for (auto [dr, dc, _] : rules_) {
    int nr = r + dr, nc = c + dc;
    if (0 <= nr && nr < H_ && 0 <= nc && nc < W_)
      out.emplace_back(nr, nc);
  }
  return out;
}

bool Moves::legalFor(std::pair<int, int> src, std::pair<int, int> dst,
                     bool firstMove, bool capture) const {
  int dr = dst.first - src.first;
  int dc = dst.second - src.second;

  for (auto [rdr, rdc, tag] : rules_) {
    if (dr != rdr || dc != rdc)
      continue;

    if (tag == 0 && !firstMove)
      continue;
    if (tag == 1 && capture)
      continue;
    if (tag == 2 && !capture)
      continue;
    return true;
  }
  return false;
}
