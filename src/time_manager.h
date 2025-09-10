#pragma once

#include "position.h"
#include "search.h"
#include <algorithm>
#include "misc.h"

struct SearchInfo;
struct ThreadData;

// Calculate how much time to spend on searching a move
inline void Optimum(int time, int inc) {
    // If ccrl sent us a negative time just assume we have a workable amount of time to search for a move
    if (time < 0) time = 1000;
    // Reserve some time overhead to avoid timing out in the engine-gui communication process
    const int safety_overhead = std::min(25, time / 2);
    // if we received a movetime command we need to spend exactly that amount of time on the move, so we don't scale
    if (info.movetimeset) {
        info.stoptimeMax = info.starttime + time - safety_overhead;
        info.stoptimeOpt = info.starttime + time - safety_overhead;
        return;
    }
    const bool cyclicTC = info.movestogo != 0;
    const int movesToGo  = cyclicTC ? std::min(info.movestogo, 50) : 50;
    const auto timeLeft = std::max(1, time + inc * (movesToGo - 1) - safety_overhead * (2 + movesToGo));
    double optScale = 0;
    if(cyclicTC){
        optScale = std::min(0.90 / movesToGo, 0.88 * time / double(timeLeft));
    }
    else{
        optScale = std::min(optScaleFixed() / 1000.0, optScaleTimeLeft() / 1000.0 * time / double(timeLeft));
    }
    // optime is the time we use to stop if we just cleared a depth
    const auto optime =  optScale * timeLeft;
    info.stoptimeBaseOpt = optime;
    info.stoptimeOpt = info.starttime + info.stoptimeBaseOpt;
    // Never use more than 76% of the total time left for a single move
    const auto maxtime = 0.76 * time - safety_overhead;
    info.stoptimeMax = info.starttime + maxtime;
}

[[nodiscard]] inline bool StopEarly() {
    // check if we used all the nodes/movetime we had or if we used more than our lowerbound of time
    return (info.timeset || info.movetimeset) && GetTimeMs() > info.stoptimeOpt;
}

[[nodiscard]] inline bool NodesOver(ThreadData* td) {
    // check if we used all the nodes/movetime we had or if we used more than our lowerbound of time
    return info.nodeset && td->nodes >= info.nodeslimit;
}

[[nodiscard]] inline bool TimeOver(ThreadData* td) {
    // check if more than Maxtime passed and we have to stop
    return NodesOver(td) || ((info.timeset || info.movetimeset)
                               && ((td->nodes & 1023) == 1023)
                               && GetTimeMs() > info.stoptimeMax);
}

inline void ScaleTm(ThreadData* td, const int bestMoveStabilityFactor, const int evalStabilityFactor) {
    const double bestmoveScale[5] = {bmScale1() / 100.0, bmScale2() / 100.0, bmScale3() / 100.0, bmScale4() / 100.0, bmScale5() / 100.0};
    const double evalScale[5] = {evalScale1() / 100.0, evalScale2() / 100.0, evalScale3() / 100.0, evalScale4() / 100.0, evalScale5() / 100.0};
    const int bestmove = GetBestMove();
    // Calculate how many nodes were spent on checking the best move
    const double bestMoveNodesFraction = static_cast<double>(nodeSpentTable[FromTo(bestmove)]) / static_cast<double>(td->nodes);
    const double nodeScalingFactor = (nodeTmBase() / 100.0 - bestMoveNodesFraction) * (nodeTmMultiplier() / 100.0);
    const double bestMoveScalingFactor = bestmoveScale[bestMoveStabilityFactor];
    const double evalScalingFactor = evalScale[evalStabilityFactor];
    // Scale the search time based on how many nodes we spent and how the best move changed
    info.stoptimeOpt = std::min<uint64_t>(info.starttime + info.stoptimeBaseOpt * nodeScalingFactor * bestMoveScalingFactor * evalScalingFactor, info.stoptimeMax);
}
