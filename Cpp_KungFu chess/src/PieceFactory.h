#pragma once
#include "Board.h"
#include "GraphicsFactory.h"
#include "Img.h"
#include "Moves.h"
#include "PhysicsFactory.h"
#include "Piece.h"
#include "State.h"
#include "json.hpp" // nlohmann::json
#include <filesystem>
#include <fstream>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class PieceFactory {
public:
  explicit PieceFactory(Board &b) : board_(b), physFactory_(b) {}

  /* scan “pieces/” once ------------------------------------------------ */
  void generateLibrary(const std::filesystem::path &root) {
    for (auto &dir : std::filesystem::directory_iterator(root))
      if (dir.is_directory())
        templates_[dir.path().filename().string()] =
            buildStateMachine(dir.path());
  }

  /* create piece ------------------------------------------------------- */
  std::unique_ptr<Piece> createPiece(const std::string &type,
                                     std::pair<int, int> cell) {
    auto it = templates_.find(type);
    if (it == templates_.end())
      throw std::runtime_error("Unknown piece type: " + type);

    State *idleTmpl = it->second.get();
    auto sharedPhys = physFactory_.create(cell, nlohmann::json{});

    std::unordered_map<const State *, std::shared_ptr<State>> clones;
    std::stack<const State *> st;
    st.push(idleTmpl);

    while (!st.empty()) {
      const State *orig = st.top();
      st.pop();
      if (clones.count(orig))
        continue;

      auto copy = std::make_shared<State>(orig->moves, orig->graphics.clone(),
                                          sharedPhys);
      copy->name = orig->name;
      clones[orig] = copy;

      for (auto &kv : orig->transitions())
        st.push(kv.second);
    }
    for (auto &[orig, copy] : clones)
      for (auto &kv : orig->transitions())
        copy->setTransition(kv.first, clones[kv.second].get());

    std::string id = type + "_(" + std::to_string(cell.first) + "," +
                     std::to_string(cell.second) + ")";
    return std::make_unique<Piece>(id, clones[idleTmpl].get());
  }

private:
  std::shared_ptr<State> buildStateMachine(const std::filesystem::path &dir) {
    auto boardSize = std::make_pair(board_.Wcells, board_.Hcells);
    auto cellPx = std::make_pair(board_.cellWpx, board_.cellHpx);

    std::unordered_map<std::string, std::shared_ptr<State>> states;

    for (auto &sdir : std::filesystem::directory_iterator(dir / "states")) {
      if (!sdir.is_directory())
        continue;
      std::string name = sdir.path().filename().string();

      /* config */
      nlohmann::json cfg;
      std::ifstream cfile(sdir.path() / "config.json");
      if (cfile && cfile.peek() != std::ifstream::traits_type::eof())
        cfile >> cfg;

      /* moves */
      Moves moves((sdir.path() / "moves.txt").string(), board_.Wcells,
                  board_.Hcells);

      /* graphics & physics */
      auto gfx =
          gfxFactory_.load(sdir.path() / "sprites", cfg["graphics"], cellPx);
      auto phy = physFactory_.create({0, 0}, cfg["physics"]);

      auto st = std::make_shared<State>(moves, gfx, phy);
      st->name = name;
      states[name] = st;
    }

    /* Arrived auto‑transition */
    for (auto &[name, st] : states) {
      nlohmann::json cfg;
      std::ifstream f(dir / "states" / name / "config.json");
      if (f)
        f >> cfg;
      std::string nxt = cfg.value("physics", nlohmann::json{})
                            .value("next_state_when_finished", "");
      if (!nxt.empty() && states.count(nxt))
        st->setTransition("Arrived", states[nxt].get());
    }

    /* default external transitions from idle */
    if (states.count("idle")) {
      if (states.count("move"))
        states["idle"]->setTransition("Move", states["move"].get());
      if (states.count("jump"))
        states["idle"]->setTransition("Jump", states["jump"].get());
    }
    return states.count("idle") ? states["idle"] : states.begin()->second;
  }

  Board &board_;
  PhysicsFactory physFactory_;
  GraphicsFactory gfxFactory_;
  std::unordered_map<std::string, std::shared_ptr<State>> templates_;
};
