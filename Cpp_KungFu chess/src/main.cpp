#include "Board.h"
#include "Game.h"
#include "Moves.h"
#include "Physics.h"
#include "Piece.h"
#include "State.h"
#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
  try {
    /* ────────── board bitmap ────────── */
    Img boardImg;
    boardImg.read("C:/Users/user1/Desktop/Work/CMakeProject1/src/board.png");

    if (boardImg.mat().channels() == 3) // 3‑ch → 4‑ch
      cv::cvtColor(boardImg.mat(), boardImg.mat(), cv::COLOR_BGR2BGRA);

    constexpr int Wcells = 8, Hcells = 8;
    int cellH = boardImg.mat().rows / Hcells;
    int cellW = boardImg.mat().cols / Wcells;
    Board board{cellH, cellW, Wcells, Hcells, boardImg};

    /* ────────── pawn sprite ─────────── */
    Img sprite;
    sprite.read("C:/PythonProject/pieces/PW/states/idle/sprites/1.png",
                {cellH, cellW}, /*keepAspect=*/false);

    if (sprite.mat().channels() == 3) // ensure 4‑ch like board
      cv::cvtColor(sprite.mat(), sprite.mat(), cv::COLOR_BGR2BGRA);

    Graphics gfx({sprite});

    /* ────────── move list (idle+fwd) ─ */
    Moves pawnMoves({{-1, 0, 1}, {0, 0, -1}}, Wcells, Hcells);

    /* ────────── physics & states ───── */
    Physics pawPhys({6, 0}, board);
    State pawIdle(pawnMoves, gfx, &pawPhys);
    State pawMove(pawnMoves, gfx, &pawPhys);
    pawIdle.setTransition("Move", &pawMove);
    pawMove.setTransition("Arrived", &pawIdle);

    Physics kwPhys({7, 4}, board);
    State kwIdle(pawnMoves, gfx, &kwPhys);
    Physics kbPhys({0, 4}, board);
    State kbIdle(pawnMoves, gfx, &kbPhys);

    /* ────────── pieces ─────────────── */
    Piece pawn("PW_(6,0)", &pawIdle);
    Piece kw("KW_(7,4)", &kwIdle);
    Piece kb("KB_(0,4)", &kbIdle);

    /* ────────── game ───────────────── */
    Game g({&pawn, &kw, &kb}, board);

    /* ────────── test move ──────────── */
    Command mv{100, "PW_(6,0)", "Move", {std::pair<int, int>{5, 0}}};
    g.processInput(mv);
    pawn.update(400);

    /* ────────── draw pieces ────────── */
    for (Piece *p : {&pawn, &kw, &kb}) {
      auto [r, c] = p->state->physics->startCell;
      int y = r * cellH, x = c * cellW;
      p->state->graphics.getImg().drawOn(board.img, x, y);
    }

    /* ────────── show result ────────── */
    board.img.show();
    cv::waitKey(0);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Fatal: " << e.what() << '\n';
    return 1;
  }
}
