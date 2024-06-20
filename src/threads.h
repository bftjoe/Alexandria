#pragma once

#include "../fixed_vector.hpp"
#include <cstdint>
#include <thread>
#include "search.h"

using namespace fixed_containers;

enum state {
    Idle,
    Search,
};
// global vector of search threads
inline FixedVector<std::thread, Threads> threads;
// global vector of thread_datas
inline FixedVector<ThreadData, Threads> threads_data;

[[nodiscard]] inline uint64_t GetTotalNodes() {
    uint64_t nodes = 0ULL;
    for (const auto& td : threads_data) {
        nodes += td.info.nodes;
    }
    return nodes;
}

inline void StopHelperThreads() {
    // Stop helper threads
    for (auto& td : threads_data) {
        td.info.stopped = true;
    }

    for (auto& th : threads) {
        if (th.joinable())
            th.join();
    }

    threads.clear();
}
