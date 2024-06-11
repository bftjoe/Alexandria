#pragma once

#include "pgm/pgm_index_dynamic.hpp"
#include "position.h"
#include "types.h"
#include <vector>

// 8 bytes:
// 2 for move
// 2 for score
// 2 for eval
// 1 for depth
// 1 for age + bound + PV
PACK(struct TTEntry {
    int16_t move = NOMOVE;
    int16_t score = SCORE_NONE;
    int16_t eval = SCORE_NONE;
    uint8_t depth = 0;
    uint8_t ageBoundPV = HFNONE; // lower 2 bits is bound, 3rd bit is PV, next 5 is age
});

static_assert(sizeof(TTEntry) == 8);
struct TTable {
    pgm::DynamicPGMIndex<uint64_t, TTEntry> pTable;
    uint8_t age;
};

extern TTable TT;

constexpr uint8_t MAX_AGE = 1 << 5; // must be power of 2
constexpr uint8_t AGE_MASK = MAX_AGE - 1;

void ClearTT();
// Initialize an TT of size MB
void InitTT(uint64_t MB);

[[nodiscard]] bool ProbeTTEntry(const ZobristKey posKey, TTEntry* tte);

void StoreTTEntry(const ZobristKey key, const int16_t move, int score, int eval, const int bound, const int depth, const bool pv, const bool wasPV);

[[nodiscard]] int GetHashfull();

void TTPrefetch(const ZobristKey posKey);

int ScoreToTT(int score, int ply);

int ScoreFromTT(int score, int ply);

int16_t MoveToTT(Move move);

Move MoveFromTT(Position *pos, int16_t packed_move);

uint8_t BoundFromTT(uint8_t ageBoundPV);

bool FormerPV(uint8_t ageBoundPV);

uint8_t AgeFromTT(uint8_t ageBoundPV);

uint8_t PackToTT(uint8_t bound, bool wasPV, uint8_t age);

void UpdateTableAge();
