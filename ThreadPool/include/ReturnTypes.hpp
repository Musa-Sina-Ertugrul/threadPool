#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include <iterator>

using namespace std;
struct Parameters
{
    int* result;
    vector<int>::iterator begin;
    vector<int>::iterator end;
    Parameters(vector<int>::iterator,vector<int>::iterator);
    ~Parameters();
};

Parameters::Parameters(vector<int>::iterator begin,vector<int>::iterator end){
    this->result = new int(0);
    this->begin = begin;
    this->end = end;
}

Parameters::~Parameters(){

}