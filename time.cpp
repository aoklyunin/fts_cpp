//
// Created by alex on 23.01.18.
//

#include "time.h"

int64_t epoch_usec() {
    auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
    return us.count();
}
