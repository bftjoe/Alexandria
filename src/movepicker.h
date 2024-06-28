#pragma once
#include "search.h"
#include <cstdint>

struct MoveList;

enum {
    PICK_TT,
    GEN_NOISY,
    PICK_GOOD_NOISY,
    PICK_KILLER,
    PICK_COUNTER,
    GEN_QUIETS,
    PICK_QUIETS,
    GEN_BAD_NOISY,
    PICK_BAD_NOISY
};

enum MovepickerType : uint8_t {
    SEARCH,
    QSEARCH
};

struct Movepicker {
    MovepickerType movepickerType;
    Position& pos;
    SearchData* sd;
    SearchStack* ss;
    MoveList moveList;
    MoveList badCaptureList;
    Move ttMove;
    Move killer;
    Move counter;
    int idx;
    int stage;

    Movepicker(Position& position, SearchData* searchdata, SearchStack* searchstack, const Move ttM, const MovepickerType movepickerT) : pos(position) {
        const Move searchKiller = searchstack->searchKiller;
        const Move counterM = searchdata->counterMoves[FromTo((searchstack - 1)->move)];

        movepickerType = movepickerT;
        sd = searchdata;
        ss = searchstack;
        ttMove = ttM;
        idx = 0;
        stage = ttMove ? PICK_TT : GEN_NOISY;
        killer = searchKiller != ttMove ? searchKiller : NOMOVE;
        counter = counterM != ttMove && counterM != killer ? counterM : NOMOVE;
    }
};

Move NextMove(Movepicker& mp, const bool skip);

