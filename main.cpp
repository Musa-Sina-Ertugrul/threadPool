#include srcThreadPool
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
#include srcReturnTypes
#include <iomanip>
#include <math.h>

using namespace std;
int recursiveSumNormal(vector<int>::iterator begin, vector<int>::iterator end);
void recursiveSumParallelPrivate(vector<int>::iterator begin, vector<int>::iterator end);
void recursiveSumParallel(Parameters parameters);
int recursiveSumParallelPrivate2(vector<int>::iterator begin, vector<int>::iterator end);
void recursiveSumParallel2(Parameters parameters);
atomic_bool accesableResult = true;
atomic_bool accesablePtr = true;
atomic<int> result = 0;
int main()
{
    result = 0;
    // ThreadPool<void, Parameters> tp(100);
    int threadNum = 0;
    char c = ' ';
    string tmp = threadNumber;
    int index = 0;
    int len = tmp.length();
    while (c != '\0')
    {
        c = tmp[index];
        if (c == '\0')
        {
            break;
        }
        threadNum += (c - '0') * pow(10, len - index - 1);
        index++;
    }
    vector<int> numbers(threadNum * 10000000);
    for (int i = 0; i < threadNum * 10000000; i++)
    {
        numbers[i] = i;
    }
    // tp.push(&recursiveSumParallel);
    auto start = chrono::high_resolution_clock::now();
    int num = recursiveSumNormal(numbers.begin(), numbers.end() - 1);
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " answer " << num << endl;
    Parameters params(numbers.begin(), numbers.end());
    /*start = chrono::high_resolution_clock::now();
    tp.run(params);
    finish = chrono::high_resolution_clock::now();
    cout << setprecision(6) << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " answer " << result.load() << endl;
    result = 0;*/
    start = chrono::high_resolution_clock::now();
    recursiveSumParallel2(params);
    finish = chrono::high_resolution_clock::now();
    cout << setprecision(6) << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " answer " << result.load() << endl;
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
/*
void recursiveSumParallel(Parameters parameters)
{
    unsigned int length = parameters.end - parameters.begin;
    unsigned int tmplength = length / static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER);
    if (length < 1)
    {

        int tmpIndex = 0;

        while (tmpIndex != length)
        {
            ThreadPool<void, Parameters>::pool[tmpIndex] = thread(ref(recursiveSumParallelPrivate), parameters.begin + tmpIndex, parameters.begin + tmpIndex + 1);
            ThreadPool<void, Parameters>::pool[tmpIndex].detach();
            tmpIndex++;
        }
        return;
    }
    for (unsigned int i = 0; i < static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER); i++)
    {

        ThreadPool<void, Parameters>::pool[i] = thread(ref(recursiveSumParallelPrivate), parameters.begin + i * tmplength, parameters.begin - 1 + (i + 1) * tmplength);
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
        ThreadPool<void, Parameters>::pool[tmp] = thread(recursiveSumParallelPrivate, parameters.begin + tmp - 1, parameters.begin + tmp);
        ThreadPool<void, Parameters>::pool[tmp].detach();
        tmp -= 2;
    }
    return;
}
*/
void recursiveSumParallel2(Parameters parameters)
{
    unsigned int length = parameters.end - parameters.begin;
    unsigned int tmplength = length / static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER);
    for (unsigned int i = 0; i < static_cast<unsigned int>(ThreadPool<void, Parameters>::THREAD_NUMBER); i++)
    {

        auto tmp = async(std::launch::async, ref(recursiveSumParallelPrivate2), parameters.begin + i * tmplength, parameters.begin - 1 + (i + 1) * tmplength);
        result += tmp.get();
    }
}
/*
void recursiveSumParallelPrivate(vector<int>::iterator begin, vector<int>::iterator end)
{
    unsigned int mid = static_cast<unsigned int>(end - begin);

    if (mid < 0)
    {
        return;
    }
    else if (mid == 0)
    {
        result.exchange(result.load(memory_order::memory_order_relaxed) + *begin, memory_order::memory_order_relaxed);
        return;
    }
    else if (mid == 1)
    {
        result.exchange(result.load() + *begin + *(begin + 1));
        return;
    }
    recursiveSumParallelPrivate(begin, begin + (mid / 2));
    recursiveSumParallelPrivate(begin + (mid / 2) + 1, end);
}*/
int recursiveSumParallelPrivate2(vector<int>::iterator begin, vector<int>::iterator end)
{
    unsigned int mid = static_cast<unsigned int>(end - begin);

    if (mid < 0)
    {
        return 0;
    }
    else if (mid == 0)
    {
        return *begin;
    }
    else if (mid == 1)
    {
        return *begin + *(begin + 1);
    }

    return recursiveSumParallelPrivate2(begin + (mid / 2) + 1, end) + recursiveSumParallelPrivate2(begin, begin + (mid / 2));
}