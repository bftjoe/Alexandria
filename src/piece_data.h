#pragma once

#include "types.h"

// convert ASCII character pieces to encoded constants
[[nodiscard]] constexpr int char_pieces(char p) {
    switch (p) {
        case 'P': return WP;
        case 'N': return WN;
        case 'B': return WB;
        case 'R': return WR;
        case 'Q': return WQ;
        case 'K': return WK;
        case 'p': return BP;
        case 'n': return BN;
        case 'b': return BB;
        case 'r': return BR;
        case 'q': return BQ;
        case 'k': return BK;
        default: assert(false); __builtin_unreachable();
    }
};

// Map promoted piece to the corresponding ASCII character
[[nodiscard]] constexpr char promoted_pieces(int p) {
    switch(p){
        case WQ: case BQ: return 'q';
        case WR: case BR: return 'r';
        case WB: case BB: return 'b';
        case WN: case BN: return 'n';
        default: assert(false); __builtin_unreachable();
    }
}
