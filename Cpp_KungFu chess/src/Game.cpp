#include "Game.h"
#include "PieceFactory.h"
#include <algorithm>
#include <iostream>
#include <stack>
#include <cmath>
#include <variant>

using Clock = std::chrono::steady_clock;

/* ─── ctor ─────────────────────────────────────────────────────────────── */
Game::Game(std::vector<Piece *> pieces, Board board)
    : pieces_(std::move(pieces)), board_(std::move(board)),
      startTime_(Clock::now()) {
  if (!validate(pieces_))
    throw InvalidBoard("duplicate pieces or no king");

  for (auto *p : pieces_)
    byId_[p->id] = p;

  cv::namedWindow("Kung‑Fu Chess");
  cv::setMouseCallback("Kung‑Fu Chess", mouseCb, this);
}

/* ─── public loop ──────────────────────────────────────────────────────── */
void Game::run() {
  const int startMs = static_cast<int>(gameTimeMs());
  for (auto *p : pieces_)
    p->reset(startMs);

  while (!isWin()) {
    const int now = static_cast<int>(gameTimeMs());
    for (auto *p : pieces_)
      p->update(now);

    {
      std::lock_guard lk(qMutex_);
      while (!inputQ_.empty()) {
        processInput(inputQ_.front());
        inputQ_.pop();
      }
    }

    draw();
    if (!show())
      break;
    resolveCollisions();
    checkPromotions();
  }
  announceWin();
  cv::destroyAllWindows();
}

/* ─── helpers ──────────────────────────────────────────────────────────── */
long long Game::gameTimeMs() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() -
                                                               startTime_)
      .count();
}

Board Game::cloneBoard() const {
  Img imgCopy;
  imgCopy.mat() = board_.img.mat().clone();
  return Board(board_.cellHpx, board_.cellWpx, board_.Wcells,
               board_.Hcells, imgCopy);
}

void Game::mouseCb(int e, int x, int y, int, void *selfPtr) {
  auto *self = static_cast<Game *>(selfPtr);
  const bool down = e == cv::EVENT_LBUTTONDOWN || e == cv::EVENT_RBUTTONDOWN;
  if (!down)
    return;

  const bool isJump = (e == cv::EVENT_RBUTTONDOWN);
  auto cell =
      std::make_pair(y / self->board_.cell_H_pix, x / self->board_.cell_W_pix);

  std::lock_guard lk(self->qMutex_);
  if (!self->selectedId_) { // 1st click
    if (auto it = self->pos_.find(cell); it != self->pos_.end())
      self->selectedId_ = it->second->id;
  } else {
    using Param = Command::Param; // alias for brevity
    self->inputQ_.emplace(static_cast<int>(self->gameTimeMs()),
                          *self->selectedId_, isJump ? "Jump" : "Move",
                          std::vector<Command::Param>{Command::Param{cell}}
                          // explicit variant
    );
  }
}

/* ─── draw / show ──────────────────────────────────────────────────────── */
void Game::draw() {
  currentBoard_ = cloneBoard();
  pos_.clear();

  const int now = static_cast<int>(gameTimeMs());
  for (auto *p : pieces_) {
    p->drawOnBoard(currentBoard_, now);
    pos_[p->state->physics->startCell] = p;
  }
}

bool Game::show() {
  cv::imshow("Kung‑Fu Chess", currentBoard_.img.mat());
  return (cv::waitKey(1) & 0xFF) != 27; // ESC quits
}

/* ─── legality & execution ─────────────────────────────────────────────── */
bool Game::pathIsClear(std::pair<int, int> a, std::pair<int, int> b) const {
  auto [ar, ac] = a;
  auto [br, bc] = b;
  int dr = (br - ar) ? ((br - ar) / std::abs(br - ar)) : 0;
  int dc = (bc - ac) ? ((bc - ac) / std::abs(bc - ac)) : 0;
  for (int r = ar + dr, c = ac + dc; std::make_pair(r, c) != b;
       r += dr, c += dc)
    if (pos_.count({r, c}))
      return false;
  return true;
}

void Game::processInput(const Command &cmd) {
  auto it = byId_.find(cmd.pieceId);
  if (it == byId_.end()) {
    std::cerr << "[DBG] unknown id\n";
    return;
  }

  Piece *mover = it->second;
  auto src = mover->state->physics->startCell;
  auto dest = std::get<std::pair<int, int>>(cmd.params[0]);

    State *cand = mover->state->nextState(cmd.type);
  if (!cand)
    cand = mover->state;
  if (auto jt = mover->state->transitions.find(cmd.type);
      jt != mover->state->transitions.end())
    cand = jt->second;

  bool capture = pos_.count(dest);
  bool firstMove = !mover->hasMoved;
  bool legalOff = cand->moves.legalFor(src, dest, firstMove, capture);
  if (cmd.type == "Move" && legalOff)
    mover->hasMoved = true;

  const Piece *occ = capture ? pos_.at(dest) : nullptr;
  bool friendly = occ && occ->id[1] == mover->id[1];
  bool clear = strchr("RBQP", mover->id[0]) ? pathIsClear(src, dest) : true;

  std::cout << "[DBG] " << cmd.type << " " << src.first << ',' << src.second
            << " -> " << dest.first << ',' << dest.second << " off=" << legalOff
            << " fr=" << friendly << " clr=" << clear << "\n";

  if (legalOff && clear && !friendly) {
    mover->state = cand;
    mover->state->reset(cmd);
  } else {
    std::cerr << "[FAIL] move rejected\n";
  }
}

/* ─── collisions / promotions / win ────────────────────────────────────── */
void Game::resolveCollisions() {
  std::unordered_map<std::pair<int, int>, std::vector<Piece *>, PairHash> occ;
  for (auto *p : pieces_)
    occ[p->state->physics->startCell].push_back(p);

  for (auto &[cell, list] : occ) {
    if (list.size() < 2)
      continue;
    auto winner =
        *std::max_element(list.begin(), list.end(), [](Piece *a, Piece *b) {
          return a->state->physics->startMs < b->state->physics->startMs;
        });
    for (auto *p : list)
      if (p != winner && p->state->physics->canBeCaptured()) {
        pieces_.erase(std::remove(pieces_.begin(), pieces_.end(), p),
                      pieces_.end());
        byId_.erase(p->id);
      }
  }
}

void Game::checkPromotions() {
  std::vector<std::pair<Piece *, std::string>> promos;
  for (auto *p : pieces_) {
    if (p->id[0] != 'P')
      continue;
    int row = p->state->physics->startCell.first;
    if (p->id[1] == 'W' && row == 0)
      promos.push_back({p, "QW"});
    else if (p->id[1] == 'B' && row == board_.Hcells - 1)
      promos.push_back({p, "QB"});
  }
  for (auto [pawn, code] : promos)
    promote(pawn, code);
}

void Game::promote(Piece *pawn, const std::string &code) {
  PieceFactory f(board_);
  auto q = f.createPiece(code, pawn->state->physics->startCell);
  std::cout << "[PROMOTION] " << pawn->id << " -> " << q->id << "\n";
  pieces_.erase(std::remove(pieces_.begin(), pieces_.end(), pawn),
                pieces_.end());
  byId_.erase(pawn->id);
  byId_[q->id] = q.get();
  pieces_.push_back(q.release());
}

/* ─── validate / win / announce ───────────────────────────────────────── */
bool Game::validate(const std::vector<Piece *> &pcs) const {
  std::unordered_set<std::pair<int, int>, PairHash> seen;
  bool wk = false, bk = false;
  for (auto *p : pcs) {
    if (!seen.emplace(p->state->physics->startCell).second)
      return false;
    wk |= p->id.rfind("KW", 0) == 0;
    bk |= p->id.rfind("KB", 0) == 0;
  }
  return wk && bk;
}

bool Game::isWin() const {
  int kings = std::count_if(pieces_.begin(), pieces_.end(),
                            [](Piece *p) { return p->id.rfind("K", 0) == 0; });
  return kings < 2;
}

void Game::announceWin() const {
  bool blackWins = std::any_of(pieces_.begin(), pieces_.end(), [](Piece *p) {
    return p->id.rfind("KB", 0) == 0;
  });
  std::cout << (blackWins ? "Black wins!\n" : "White wins!\n");
}
