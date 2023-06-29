#include "ThreadPool.hpp"
#include <vector>
#include <chrono>
#include <iostream>
#include <cstdarg>
#include <memory>
#include <functional>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "ReturnTypes.hpp"
#include <iomanip>

using namespace std;
int recursiveSumNormal(vector<int>::iterator begin, vector<int>::iterator end);
void recursiveSumParallelPrivate(int *result, vector<int>::iterator begin, vector<int>::iterator end);
void recursiveSumParallel(Parameters parameters);
atomic_bool accesableResult = true;
atomic_bool accesablePtr = true;
int main()
{
    ThreadPool<void, Parameters> tp(100);
    vector<int> numbers(100000);
    for (int i = 0; i < 100000; i++)
    {
        numbers[i] = i;
    }
    tp.push(&recursiveSumParallel);
    auto start = chrono::high_resolution_clock::now();
    int num = recursiveSumNormal(numbers.begin(), numbers.end() - 1);
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " answer " << num << endl;
    Parameters params(numbers.begin(), numbers.end());
    start = chrono::high_resolution_clock::now();
    tp.run(params);
    finish = chrono::high_resolution_clock::now();
    cout << setprecision(6) << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " answer " << *params.result << endl;
    return 0;
}
int recursiveSumNormal(vector<int>::iterator begin, vector<int>::iterator end)
{
    unsigned int mid = static_cast<unsigned int>(end - begin) / 2;
    if (end - begin < 0)
    {
        return 0;
    }
    else if (end - begin == 0)
    {
        int tmp = *begin;
        return tmp;
    }
    else if (end - begin == 1)
    {
        int tmp = *begin + *(begin + 1);
        return tmp;
    }
    int tmp = recursiveSumNormal(begin, begin + mid) + recursiveSumNormal(begin + mid + 1, end);
    return tmp;
}
void recursiveSumParallel(Parameters parameters)
{
    unsigned int length = parameters.end - parameters.begin;
    unsigned int tmplength = length / static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER);
    if (length < 1)
    {

        int tmpIndex = 0;

        while (tmpIndex != length)
        {
            ThreadPool<void, Parameters>::pool[tmpIndex] = thread(ref(recursiveSumParallelPrivate), parameters.result, parameters.begin + tmpIndex, parameters.begin + tmpIndex + 1);
            ThreadPool<void, Parameters>::pool[tmpIndex].detach();
            tmpIndex++;
        }
        return;
    }
    for (unsigned int i = 0; i < static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER); i++)
    {

        ThreadPool<void, Parameters>::pool[i] = thread(ref(recursiveSumParallelPrivate), parameters.result, parameters.begin + i * tmplength, parameters.begin - 1 + (i + 1) * tmplength);
        ThreadPool<void, Parameters>::pool[i].detach();
    }
    int tmp = tmplength * static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER);
    tmp = length - tmp;
    if (tmp == 0)
    {
        return;
    }
    while (tmp != 0)
    {
        ThreadPool<void, Parameters>::pool[tmp] = thread(recursiveSumParallelPrivate, parameters.result, parameters.begin + tmp - 1, parameters.begin + tmp);
        ThreadPool<void, Parameters>::pool[tmp].detach();
        tmp -= 2;
    }
    return;
}

void recursiveSumParallelPrivate(int *result, vector<int>::iterator begin, vector<int>::iterator end)
{
    mutex m;
    unique_lock<mutex> tmplock(m);
    condition_variable cv;
    cv.wait(tmplock, []() -> bool
            { return accesablePtr.load(); });
    accesablePtr.exchange(false );
    unsigned int mid = static_cast<unsigned int>(end - begin);
    accesablePtr.exchange(true);
    
    if (mid < 0)
    {
        return;
    }
    else if (mid == 0)
    {

        mutex m;
        unique_lock<mutex> lock(m);
        condition_variable cv;
        cv.wait(lock, []() -> bool
                { return accesableResult.load(); });
        accesableResult.exchange(false);
        *result += *begin;
        accesableResult.exchange(true);
        return;
    }
    else if (mid == 1)
    {
        mutex m;
        unique_lock<mutex> lock(m);
        condition_variable cv;
        cv.wait(lock, []() -> bool
                { return accesableResult.load(); });
        accesableResult.exchange(false);
        *result += *begin + *(begin + 1);
        accesableResult.exchange(true);
        return;
    }
    recursiveSumParallelPrivate(result, begin, begin + (mid/2));
    recursiveSumParallelPrivate(result, begin + (mid/2) + 1, end);
}