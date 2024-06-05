#include "nnue.h"
#include "simd.h"
#include <algorithm>
#include "position.h"
#include <cstdio>
#include <cstring>
#include <iostream>

// Thanks to Disservin for having me look at his code and Luecx for the
// invaluable help and the immense patience

void NNUE::accumulate(NNUE::Accumulator& board_accumulator, Position* pos) {
    for (int i = 0; i < L1_SIZE; i++) {
        board_accumulator.values[0][i] = FTBiases[i];
        board_accumulator.values[1][i] = FTBiases[i];
    }

    for (int i = 0; i < 64; i++) {
        bool input = pos->pieces[i] != EMPTY;
        if (!input) continue;
        auto [whiteIdx, blackIdx] = GetIndex(pos->pieces[i], i);
        auto whiteAdd = &FTWeights[whiteIdx * L1_SIZE];
        auto blackAdd = &FTWeights[blackIdx * L1_SIZE];
        for (int j = 0; j < L1_SIZE; j++) {
            board_accumulator.values[0][j] += whiteAdd[j];
        }
        for (int j = 0; j < L1_SIZE; j++) {
            board_accumulator.values[1][j] += blackAdd[j];
        }
    }
}

void NNUE::update(NNUE::Accumulator *acc) {

    int adds = acc->NNUEAdd.size();
    int subs = acc->NNUESub.size();

    if (adds == 0 && subs == 0)
        return;

    if (!(acc - 1)->NNUEAdd.empty() && !(acc - 1)->NNUESub.empty())
        update(acc - 1);

    // Quiets
    if (adds == 1 && subs == 1) {
        addSub(acc, acc - 1, acc->NNUEAdd[0], acc->NNUESub[0]);
    }
    // Captures
    else if (adds == 1 && subs == 2) {
        addSubSub(acc, acc - 1, acc->NNUEAdd[0], acc->NNUESub[0], acc->NNUESub[1]);
    }
    // Castling
    else {
        addSub(acc, acc - 1, acc->NNUEAdd[0], acc->NNUESub[0]);
        addSub(acc, acc, acc->NNUEAdd[1], acc->NNUESub[1]);
        // Note that for second addSub, we put acc instead of acc - 1 because we are updating on top of
        // the half-updated accumulator
    }
    // Reset the add and sub vectors
    acc->NNUEAdd.clear();
    acc->NNUESub.clear();
}

void NNUE::addSub(NNUE::Accumulator *new_acc, NNUE::Accumulator *prev_acc, NNUEIndices add, NNUEIndices sub) {
    auto [whiteAddIdx, blackAddIdx] = add;
    auto [whiteSubIdx, blackSubIdx] = sub;
    auto whiteAdd = &FTWeights[whiteAddIdx * L1_SIZE];
    auto whiteSub = &FTWeights[whiteSubIdx * L1_SIZE];
    for (int i = 0; i < L1_SIZE; i++) {
        new_acc->values[0][i] = prev_acc->values[0][i] - whiteSub[i] + whiteAdd[i];
    }
    auto blackAdd = &FTWeights[blackAddIdx * L1_SIZE];
    auto blackSub = &FTWeights[blackSubIdx * L1_SIZE];
    for (int i = 0; i < L1_SIZE; i++) {
        new_acc->values[1][i] = prev_acc->values[1][i] - blackSub[i] + blackAdd[i];
    }
}

void NNUE::addSubSub(NNUE::Accumulator *new_acc, NNUE::Accumulator *prev_acc, NNUEIndices add, NNUEIndices sub1, NNUEIndices sub2) {

    auto [whiteAddIdx, blackAddIdx] = add;
    auto [whiteSubIdx1, blackSubIdx1] = sub1;
    auto [whiteSubIdx2, blackSubIdx2] = sub2;

    auto whiteAdd = &FTWeights[whiteAddIdx * L1_SIZE];
    auto whiteSub1 = &FTWeights[whiteSubIdx1 * L1_SIZE];
    auto whiteSub2 = &FTWeights[whiteSubIdx2 * L1_SIZE];
    for (int i = 0; i < L1_SIZE; i++) {
        new_acc->values[0][i] = prev_acc->values[0][i] - whiteSub1[i] - whiteSub2[i] + whiteAdd[i];
    }
    auto blackAdd = &FTWeights[blackAddIdx * L1_SIZE];
    auto blackSub1 = &FTWeights[blackSubIdx1 * L1_SIZE];
    auto blackSub2 = &FTWeights[blackSubIdx2 * L1_SIZE];
    for (int i = 0; i < L1_SIZE; i++) {
        new_acc->values[1][i] = prev_acc->values[1][i] - blackSub1[i] - blackSub2[i] + blackAdd[i];
    }
}

int32_t NNUE::ActivateFTAndAffineL1(const int16_t *us, const int16_t *them, const int16_t *weights, const int16_t bias) {
    #if defined(USE_SIMD)
    vepi32 sum  = vec_zero_epi32();
    const vepi16 Zero = vec_zero_epi16();
    const vepi16 One  = vec_set1_epi16(FT_QUANT);
    int weightOffset = 0;
    for (const int16_t *acc : {us, them}) {
        for (int i = 0; i < L1_SIZE; i += CHUNK_SIZE) {
            vepi16 input   = vec_loadu_epi(reinterpret_cast<const vepi16*>(&acc[i]));
            vepi16 weight  = vec_loadu_epi(reinterpret_cast<const vepi16*>(&weights[i + weightOffset]));
            vepi16 clipped = vec_min_epi16(vec_max_epi16(input, Zero), One);

            // In squared clipped relu, we want to do (clipped * clipped) * weight.
            // However, as clipped * clipped does not fit in an int16 while clipped * weight does,
            // we instead do mullo(clipped, weight) and then madd by clipped.
            vepi32 product = vec_madd_epi16(vec_mullo_epi16(clipped, weight), clipped);
            sum = vec_add_epi32(sum, product);
        }

        weightOffset += L1_SIZE;
    }

    return (vec_reduce_add_epi32(sum) / FT_QUANT + bias) * NET_SCALE / (FT_QUANT * L1_QUANT);

    #else
    int sum = 0;
    int weightOffset = 0;
    for (const int16_t *acc : {us, them}) {
        for (int i = 0; i < L1_SIZE; ++i) {
            int16_t input   = acc[i];
            int16_t weight  = weights[i + weightOffset];
            int16_t clipped = std::clamp(input, int16_t(0), int16_t(FT_QUANT));
            sum += static_cast<int16_t>(clipped * weight) * clipped;
        }

        weightOffset += L1_SIZE;
    }

    return (sum / FT_QUANT + bias) * NET_SCALE / (FT_QUANT * L1_QUANT);
    #endif
}

int32_t NNUE::output(const NNUE::Accumulator& board_accumulator, const bool whiteToMove, const int outputBucket) {
    // this function takes the net output for the current accumulators and returns the eval of the position
    // according to the net
    const int16_t* us;
    const int16_t* them;
    if (whiteToMove) {
        us = board_accumulator.values[0].data();
        them = board_accumulator.values[1].data();
    } else {
        us = board_accumulator.values[1].data();
        them = board_accumulator.values[0].data();
    }

    const int32_t bucketOffset = 2 * L1_SIZE * outputBucket;
    return ActivateFTAndAffineL1(us, them, &L1Weights[bucketOffset], L1Biases[outputBucket]);
}

NNUEIndices NNUE::GetIndex(const int piece, const int square) {
    constexpr std::size_t COLOR_STRIDE = 64 * 6;
    constexpr std::size_t PIECE_STRIDE = 64;
    int piecetype = GetPieceType(piece);
    int color = Color[piece];
    std::size_t whiteIdx = color * COLOR_STRIDE + piecetype * PIECE_STRIDE + (square ^ 0b111'000);
    std::size_t blackIdx = (1 ^ color) * COLOR_STRIDE + piecetype * PIECE_STRIDE + square;
    return {whiteIdx, blackIdx};
}
