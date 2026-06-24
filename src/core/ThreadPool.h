#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
    template<class F>
    auto enqueue(F&& f) -> std::future<decltype(f())>;
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
};

inline ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i)
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock lock(queueMutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
}
inline ThreadPool::~ThreadPool() {
    { std::unique_lock lock(queueMutex); stop = true; }
    condition.notify_all();
    for (auto& w : workers) w.join();
}
template<class F>
auto ThreadPool::enqueue(F&& f) -> std::future<decltype(f())> {
    using R = decltype(f());
    auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(f));
    std::future<R> res = task->get_future();
    {
        std::unique_lock lock(queueMutex);
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}
