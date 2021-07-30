#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    ThreadPool(size_t);
    template <class F, class... Args>
    auto enqueue(F&& Func, Args&&... ParamArgs) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> m_workers;
    // the task queue
    std::queue<std::function<void()>> m_tasks;

    // synchronization
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t Threads)
        : m_stop(false)
{
    for (size_t i = 0; i < Threads; ++i)
        m_workers.emplace_back([this] {
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->m_queueMutex);
                    this->m_condition.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });
                    if (this->m_stop && this->m_tasks.empty())
                        return;
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }

                task();
            }
        });
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F&& Func, Args&&... ParamArgs) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(Func), std::forward<Args>(ParamArgs)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        // don't allow enqueueing after stopping the pool
        if (m_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        m_tasks.emplace([task]() { (*task)(); });
    }
    m_condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (std::thread& worker : m_workers)
        worker.join();
}
