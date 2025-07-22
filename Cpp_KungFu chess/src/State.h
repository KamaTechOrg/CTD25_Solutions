// State.h
#pragma once
#include "Graphics.h"
#include "Moves.h"
#include "Physics.h"
#include <map>

class State {
public:
  State(Moves mv, Graphics gfx, Physics *phys);

  void setTransition(const std::string &ev, State *target);

  void reset(const Command &cmd); // called on external cmd
  bool canTransition(long long now) const;
  State *getNext(const Command &cmd, long long now);

  State *update(long long now); // physics+gfx advance

  // exposed for Piece/Game
  Moves moves;
  Graphics graphics;
  Physics *physics;
  long long cooldownEndMs{0};
  std::string name{"state"};

  /* add inside public section */
  State *nextState(const std::string &ev) const {
    auto it = trans_.find(ev);
    return it != trans_.end() ? it->second : nullptr;
  }
  const std::map<std::string, State *> &transitions() const { return trans_; }


private:
  std::map<std::string, State *> trans_;
};

