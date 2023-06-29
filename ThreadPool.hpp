#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <memory>
#include <utility>
#include <exception>
#include <iostream>
using namespace std;
using namespace std::chrono_literals;
template <typename T, typename Parameters>
class ThreadPool
{
private:
    int currentElementNumber;
    static atomic_bool accessable;
    static atomic<int> gateHolderIndex;
    vector<function<T(Parameters)>> duties;

public:
    static const size_t THREAD_NUMBER;
    static vector<condition_variable> gateHolders;
    static vector<thread> pool;
    static void threadClear();
    function<T(Parameters)> pop();
    void push(function<T(Parameters)>);
    void pushFront(function<T(Parameters)>);
    T run(Parameters args);
    static void threadStart();
    ThreadPool(int limit);
    ~ThreadPool();
};

template <typename T, typename Parameters>
ThreadPool<T, Parameters>::ThreadPool(int limit)
{
    this->duties.resize(limit);
    this->currentElementNumber = 0;
}
template <typename T, typename Parameters>
ThreadPool<T, Parameters>::~ThreadPool()
{
}

template <typename T, typename Parameters>
atomic_bool ThreadPool<T, Parameters>::accessable = atomic_bool(true);

template <typename T, typename Parameters>
atomic<int> ThreadPool<T, Parameters>::gateHolderIndex = 0;

template <typename T, typename Parameters>
const size_t ThreadPool<T, Parameters>::THREAD_NUMBER = thread::hardware_concurrency();

template <typename T, typename Parameters>
vector<thread> ThreadPool<T, Parameters>::pool = vector<thread>(ThreadPool<T, Parameters>::THREAD_NUMBER);

template <typename T, typename Parameters>
vector<condition_variable> ThreadPool<T, Parameters>::gateHolders = vector<condition_variable>(ThreadPool<T, Parameters>::THREAD_NUMBER);

template <typename T, typename Parameters>
void ThreadPool<T, Parameters>::threadClear()
{
    mutex m;
    unique_lock<mutex> tmpLock = unique_lock<mutex>(m);
    condition_variable tmpCV = condition_variable();
    for (size_t i(0); i != ThreadPool::THREAD_NUMBER; ++i)
    {
        auto now = chrono::system_clock::now();
        tmpCV.wait_until(tmpLock, now + 1s, [&i]() -> bool
                         { return ThreadPool<T, Parameters>::pool[i].joinable(); });
        ThreadPool<T, Parameters>::pool[i].join();
    }
}
template <typename T, typename Parameters>
void ThreadPool<T, Parameters>::threadStart()
{
    for (size_t i(0); i != ThreadPool<T, Parameters>::THREAD_NUMBER; ++i)
    {
        ThreadPool<T, Parameters>::pool[i] = thread();
    }
}
template <typename T, typename Parameters>
function<T(Parameters)> ThreadPool<T, Parameters>::pop()
{
    if (!this->accessable.load(memory_order::memory_order_relaxed))
    {
        int tmp = ThreadPool<T, Parameters>::gateHolderIndex.load();
        ThreadPool<T, Parameters>::gateHolderIndex.exchange(ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed) + 1, memory_order::memory_order_relaxed);
        mutex m;
        unique_lock<mutex> locks(m);
        ThreadPool<T, Parameters>::gateHolders[tmp].wait(locks, [this]() -> bool
                                                         { return this->accessable.load(memory_order::memory_order_relaxed); });
        ThreadPool<T, Parameters>::gateHolderIndex.exchange(ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed) - 1, memory_order::memory_order_relaxed);
    }
    ThreadPool<T, Parameters>::accessable.exchange(false, memory_order::memory_order_relaxed);
    if (this->currentElementNumber > 1)
    {
        function<T(Parameters)> tmp;
        swap(tmp,this->duties[0]);
        for (int i = 1; i != this->currentElementNumber; i++)
        {
            this->duties[i - 1] = move(this->duties[i]);
        }
        this->currentElementNumber--;
        ThreadPool<T, Parameters>::accessable.exchange(true, memory_order::memory_order_relaxed);
        return tmp;
    }
    else if (this->currentElementNumber == 1)
    {
        function<T(Parameters)> tmp;
        swap(tmp,this->duties[0]);
        this->currentElementNumber--;
        ThreadPool<T, Parameters>::accessable.exchange(true, memory_order::memory_order_relaxed);
        return tmp;
    }
    ThreadPool<T, Parameters>::accessable.exchange(true, memory_order::memory_order_relaxed);
    return nullptr;
}

template <typename T, typename Parameters>
void ThreadPool<T, Parameters>::push(function<T(Parameters)> func)
{
    if (!this->accessable.load(memory_order::memory_order_relaxed))
    {
        int tmp = ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed);
        ThreadPool<T, Parameters>::gateHolderIndex.exchange(ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed) + 1, memory_order::memory_order_relaxed);
        mutex m;
        unique_lock<mutex> locks(m);
        ThreadPool<T, Parameters>::gateHolders[tmp].wait(locks, [this]() -> bool
                                                         { return this->accessable.load(memory_order::memory_order_relaxed); });
        ThreadPool<T, Parameters>::gateHolderIndex.exchange(ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed) - 1, memory_order::memory_order_relaxed);
    }
    ThreadPool<T, Parameters>::accessable.exchange(false, memory_order::memory_order_relaxed);
    this->duties[this->currentElementNumber] = func;
    ThreadPool<T, Parameters>::accessable.exchange(true, memory_order::memory_order_relaxed);
    this->currentElementNumber++;
}

template <typename T, typename Parameters>
void ThreadPool<T, Parameters>::pushFront(function<T(Parameters)> func)
{
    if (!this->accessable.load(memory_order::memory_order_relaxed))
    {
        int tmp = ThreadPool<T, Parameters>::gateHolderIndex.load();
        ThreadPool<T, Parameters>::gateHolderIndex.exchange(ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed) + 1, memory_order::memory_order_relaxed);
        mutex m;
        unique_lock<mutex> locks(m);
        ThreadPool<T, Parameters>::gateHolders[tmp].wait(locks, [this]() -> bool
                                                         { return this->accessable.load(memory_order::memory_order_relaxed); });
        ThreadPool<T, Parameters>::gateHolderIndex.exchange(ThreadPool<T, Parameters>::gateHolderIndex.load(memory_order::memory_order_relaxed) - 1, memory_order::memory_order_relaxed);
    }
    ThreadPool<T, Parameters>::accessable.exchange(false, memory_order::memory_order_relaxed);
    for (int i = 0; i != this->currentElementNumber + 1; i++)
    {
        this->duties[i + 1] = move(this->duties[i]);
    }
    this->duties[this->currentElementNumber] = func;
    ThreadPool<T, Parameters>::accessable.exchange(true, memory_order::memory_order_relaxed);
    this->currentElementNumber++;
}

template <typename T, typename Parameters>
T ThreadPool<T, Parameters>::run(Parameters args)
{
    function<T(Parameters)> func = this->pop();
    try
    {
        func(args);
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
    }

    return;
}