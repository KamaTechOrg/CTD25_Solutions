
// Board.cpp
#include "Board.h"
Board Board::clone() const {
    Board b = *this;
    b.img = Img();                       // deep‑copy underlying pixels
    b.img.read("");                      // placeholder – supply path if needed
    b.img.mat() = img.mat().clone();
    return b;
}