#include "src/core.h"

#include <thread>
#include <vector>
#include <functional>

template <typename T>
void processVector(std::vector<T>& vec, std::function<void(T&)> func) {
    const size_t numHardwareThreads = std::min((size_t)12, (size_t)std::thread::hardware_concurrency());
    const size_t threadPoolSize = numHardwareThreads > 2 ? numHardwareThreads - 2 : 1;

    const size_t numElementsPerThread = vec.size() / threadPoolSize;

    const auto workerFunc = [&](const size_t threadId) {
        const size_t startElementIndex = numElementsPerThread * threadId;
        const size_t endIndex = threadId == threadPoolSize - 1 ? vec.size() : numElementsPerThread * (threadId + 1);
        for (size_t i = startElementIndex; i < endIndex; i++)
            func(vec[i]);
        };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < threadPoolSize; i++)
        threads.push_back(std::thread(std::bind(workerFunc, i)));

    for (auto& t : threads)
        t.join();
}

template <typename T>
void processConstVector(const std::vector<T>& vec, std::function<void(const T&)> func) {
    const size_t numHardwareThreads = std::min((size_t)12, (size_t)std::thread::hardware_concurrency());
    const size_t threadPoolSize = numHardwareThreads > 2 ? numHardwareThreads - 2 : 1;

    const size_t numElementsPerThread = vec.size() / threadPoolSize;

    const auto workerFunc = [&](const size_t threadId) {
        const size_t startElementIndex = numElementsPerThread * threadId;
        const size_t endIndex = threadId == threadPoolSize - 1 ? vec.size() : numElementsPerThread * (threadId + 1);
        for (size_t i = startElementIndex; i < endIndex; i++)
            func(vec[i]);
        };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < threadPoolSize; i++)
        threads.push_back(std::thread(std::bind(workerFunc, i)));

    for (auto& t : threads)
        t.join();
}