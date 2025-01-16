#pragma once

#include "position.h"
#include <algorithm>

// if we don't have enough material to mate consider the position a draw
[[nodiscard]] inline bool MaterialDraw(const Position* pos) {
    // If we only have kings on the board then it's a draw
    switch(pos->PieceCount()){
        // K v K
        case 2: return true;
        // KN v K or KB v K
        case 3: return (CountBits(GetPieceBB(pos, KNIGHT)) == 1) || (CountBits(GetPieceBB(pos, BISHOP)) == 1); 
        // (KNN v K, KN v KN) || KB v KB
        case 4: return (CountBits(GetPieceBB(pos, KNIGHT)) == 2) || (CountBits(GetPieceBB(pos, BISHOP)) == 2 && CountBits(pos->GetPieceColorBB(BISHOP, WHITE)) == 1);
        default: return false;
    }
}

[[nodiscard]] static inline int ScaleMaterial(const Position* pos, int eval) {
    const int knights = CountBits(GetPieceBB(pos, KNIGHT));
    const int bishops = CountBits(GetPieceBB(pos, BISHOP));
    const int rooks = CountBits(GetPieceBB(pos, ROOK));
    const int queens = CountBits(GetPieceBB(pos, QUEEN));
    const int phase = std::min(3 * knights + 3 * bishops + 5 * rooks + 10 * queens, 64);
    // Scale between [0.75, 1.00]
    return eval * (192 + phase) / 256;
}

[[nodiscard]] inline int EvalPositionRaw(Position* pos) {
    // Update accumulators to ensure we are up to date on the current board state
    nnue.update(&pos->AccumulatorTop(), pos);
    const int pieceCount = pos->PieceCount();
    const int outputBucket = std::min((63 - pieceCount) * (32 - pieceCount) / 225, 7);
    return nnue.output(pos->AccumulatorTop(), pos->side, outputBucket);
}

// position evaluation
[[nodiscard]] inline int EvalPosition(Position* pos) {
    int eval = EvalPositionRaw(pos);
    eval = ScaleMaterial(pos, eval);
    eval = eval * (200 - pos->get50MrCounter()) / 200;
    eval = (eval / 16) * 16 - 1 + (pos->posKey & 0x2);
    // Clamp eval to avoid it somehow being a mate score
    eval = std::clamp(eval, -MATE_FOUND + 1, MATE_FOUND - 1);
    return eval;
}
