#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include "search.h"

atomic<bool> stopSearch = true;
atomic<unsigned> stoppedWorkers = 0;
atomic<bool> allWorkersPaused = true;
// global vector of search threads
inline std::vector<std::thread> threads;
// global vector of thread_datas
inline std::vector<ThreadData> threads_data;

[[nodiscard]] inline uint64_t GetTotalNodes() {
    uint64_t nodes = 0ULL;
    for (const auto& td : threads_data) {
        nodes += td.info.nodes;
    }
    return nodes;
}

// Pause helper threads
inline void StopHelperThreads() {
    pauseSearch = true;
    allWorkersPaused.wait(false);
}
