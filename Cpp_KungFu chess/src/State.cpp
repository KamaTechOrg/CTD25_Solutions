
// State.cpp
#include "State.h"
#include <iostream>

State::State(Moves mv, Graphics gfx, Physics *phys)
    : moves(std::move(mv)), graphics(std::move(gfx)), physics(phys) {}

void State::setTransition(const std::string &ev, State *target) {
  trans_[ev] = target;
}

void State::reset(const Command &cmd) {
  graphics.reset(cmd);
  physics->reset(cmd);
}

bool State::canTransition(long long now) const { return now >= cooldownEndMs; }

State *State::getNext(const Command &cmd, long long now) {
  auto it = trans_.find(cmd.type);
  if (it == trans_.end())
    return this;

  if (cmd.type == "Arrived") {
    // cooldown handling identical to Python
    int rest = (name == "move") ? 6000 : (name == "jump") ? 3000 : 0;
    State *nxt = it->second;
    nxt->graphics.reset(cmd);
    if (rest) {
      nxt->physics->startMs = now;
      nxt->physics->durationMs = rest;
      nxt->physics->reset(Command{now, "?", "Idle", {}});
      nxt->cooldownEndMs = now + rest;
    } else
      nxt->cooldownEndMs = 0;
    return nxt;
  }

  if (canTransition(now)) {
    State *nxt = it->second;
    nxt->reset(cmd);
    return nxt;
  }
  return this;
}

State *State::update(long long now) {
  if (auto res = physics->update(now))
    return getNext(*res, now);
  graphics.update(now);
  return this;
}