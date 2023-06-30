#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <future>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <iostream>

using namespace std;
void initiliazer(vector<int>::iterator,vector<int>::iterator);
int main(){
    vector<int> test(800000000);
    vector<int> test2(800000000);
    unsigned int threadNum = thread::hardware_concurrency();
    int tmp = 0;
    auto start = chrono::high_resolution_clock::now();
    for(int i = 0;i<800000000;i++){
        test[i]=tmp;
        if(tmp == 100000000){
            tmp = 0;
        }
    }
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(finish-start).count()<<endl;
    start = chrono::high_resolution_clock::now();
    unsigned int tmpLength = 80000/threadNum;
    for(unsigned int i = 0;i<threadNum;i++){
        async(launch::async,ref(initiliazer),test2.begin()+i*tmpLength,test2.begin()+(i+1)*(tmpLength-1));
    }
    finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(finish-start).count()<<endl;
    return 0;
}
void initiliazer(vector<int>::iterator begin,vector<int>::iterator end){
    unsigned int length = end - begin;
    for(unsigned int i = 0;i<length;i++){
        begin[i] = i;
    }
}