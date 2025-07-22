// Board.h
#pragma once
#include "Img.h"

struct Board {
    int cellHpx, cellWpx;
    int Wcells,  Hcells;
    Img img;

    [[nodiscard]] Board clone() const;
};

