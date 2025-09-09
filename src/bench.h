#pragma once

// starts a bench for fchess, searching a set of positions up to a set depth
inline void StartBench(int depth = 14) {
    // init all
    UciOptions uciOptions;
    ThreadData* td(new ThreadData());
    uint64_t totalNodes = 0;
    InitTT(64);
    InitNewGame(td);
    auto start = std::chrono::steady_clock::now();
    for (int positions = 0; positions < 51; positions++) {
        ParseFen(benchmarkfens[positions], &td->pos);
        std::cout << "\nPosition: " << positions + 1 << " fen: " << benchmarkfens[positions] << std::endl;
        RootSearch(depth, td, &uciOptions);
        totalNodes += td->nodes;
    }
    auto end = std::chrono::steady_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "\n";
    std::cout << totalNodes << " nodes " << signed(totalNodes / (totalTime + 1) * 1000) << " nps" << std::endl;
    delete td;
}
