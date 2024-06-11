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
extern FixedVector<std::thread, Threads> threads;
// global vector of thread_datas
extern FixedVector<ThreadData, Threads> threads_data;

[[nodiscard]] uint64_t GetTotalNodes();
void StopHelperThreads();
