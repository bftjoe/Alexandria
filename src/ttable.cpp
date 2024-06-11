#include "ttable.h"
#include "io.h"
#include <iostream>
// This include breaks on non x86 target platforms
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#include "xmmintrin.h"
#endif

TTable TT;

void ClearTT() {
//    std::fill(TT.pTable.begin(), TT.pTable.end(), TTBucket());
    TT.age = 1;
}

void InitTT(uint64_t MB) {
    ClearTT();
//    std::cout << "TT init complete with " << numBuckets << " buckets and " << numBuckets * ENTRIES_PER_BUCKET << " entries\n";
}

bool ProbeTTEntry(const ZobristKey posKey, TTEntry* tte) {
  auto e = TT.pTable.find(posKey);
    if (e == TT.pTable.end() )
      return false;
  
  *tte = e->second;
  return true;   
    
}

void StoreTTEntry(const ZobristKey key, const int16_t move, int score, int eval, const int bound, const int depth, const bool pv, const bool wasPV) {
    const uint8_t TTAge = TT.age;
    TTEntry tte;
    tte.ageBoundPV = PackToTT(bound, wasPV, TTAge);
    tte.score = static_cast<int16_t>(score);
    tte.eval = static_cast<int16_t>(eval);
    tte.depth = static_cast<uint8_t>(depth);
    tte.move = move;
    TT.pTable.insert_or_assign(key, tte);  
}

int GetHashfull() {
    return 0;
}

// prefetches the data in the given address in l1/2 cache in a non blocking way.
void prefetch(const void* addr) {
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
    _mm_prefetch((char*)addr, _MM_HINT_T0);
#else
    __builtin_prefetch(addr);
#endif
}

void TTPrefetch(const ZobristKey posKey) {
//    prefetch(&TT.pTable[Index(posKey)].entries[0]);
}


int ScoreToTT(int score, int ply) {
    if (score > MATE_FOUND)
        score += ply;
    else if (score < -MATE_FOUND)
        score -= ply;

    return score;
}

int ScoreFromTT(int score, int ply) {
    if (score > MATE_FOUND)
        score -= ply;
    else if (score < -MATE_FOUND)
        score += ply;

    return score;
}

int16_t MoveToTT(Move move) {
    return (move & 0xffff);
}

Move MoveFromTT(Position *pos, int16_t packed_move) {
    // It's important to preserve a move being null even it's being unpacked
    if (packed_move == NOMOVE)
        return NOMOVE;

    const int piece = pos->PieceOn(From(packed_move));
    return (static_cast<Move>(static_cast<uint16_t>(packed_move)) | (piece << 16));
}

uint8_t BoundFromTT(uint8_t ageBoundPV) {
    return ageBoundPV & 0b00000011;
}

bool FormerPV(uint8_t ageBoundPV) {
    return ageBoundPV & 0b00000100;
}

uint8_t AgeFromTT(uint8_t ageBoundPV) {
    return (ageBoundPV & 0b11111000) >> 3;
}

uint8_t PackToTT(uint8_t bound, bool wasPV, uint8_t age) {
    return static_cast<uint8_t>(bound + (wasPV << 2) + (age << 3));
}

void UpdateTableAge() {
    TT.age = (TT.age + 1) & AGE_MASK;
}
