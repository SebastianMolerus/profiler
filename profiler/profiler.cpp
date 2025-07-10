// profiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <chrono>
#include <windows.h>
#include <fstream>

//[
//{"name": "ThreadControllerImpl::RunTask", "cat" : "perf", "ph" : "X", "ts" : 123, "dur" : 234, "pid" : 14508, "tid" : 15216}
//]

std::string create_perf_file()
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    std::stringstream ss;
    ss <<  lt.wDay << "_" << lt.wMonth << "_" << lt.wYear <<"_"<<lt.wHour << "_" << lt.wMinute << "_" << lt.wSecond << "_perf.json";

    std::ofstream ofs{ ss.str() };
    ofs << "[.";
    return ss.str();
}

std::string const file_name{ create_perf_file() };

class perf
{
public:
    perf(std::string const& func_name)
        :m_func_name{ func_name }, m_begin {std::chrono::high_resolution_clock::now()}
    {
    }

    ~perf()
    {
        auto const now{ std::chrono::high_resolution_clock::now() };

        auto const elapsed{ now - m_begin };
        auto const dur_micro_s{ std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() };
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> m_begin;
    std::string m_func_name;
};

#define PERF_IT perf __p__(__FUNCTION__);

int main()
{
    PERF_IT;
}