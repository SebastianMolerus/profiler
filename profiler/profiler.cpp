#include <iostream>
#include <sstream>
#include <chrono>
#include <windows.h>
#include <fstream>
#include <thread>
#include <unordered_map>

namespace {

    std::string create_perf_file()
    {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        std::stringstream ss;
        ss << lt.wDay << "_" << lt.wMonth << "_" << lt.wYear << "_" << lt.wHour << "_" << lt.wMinute << "_" << lt.wSecond << "_perf.json";

        std::ofstream ofs{ ss.str() };
        ofs << R"([{"args":{"name":"TestApplication"},"cat":"__metadata","name":"process_name","ph":"M","pid":0,"tid":0,"ts":0},)";
        ofs << "\n\n]";
        return ss.str();
    }

    std::string const file_name{ create_perf_file() };

    using tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
    tp get_now()
    {
        return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
    }
}

class perf
{
public:
    perf(std::string const& func_name, std::string const& category)
        :m_func_name{ func_name }, m_cat{category}
    {
        if (m_cat_to_id.find(category) == m_cat_to_id.end()) {
            m_cat_to_id[category] = m_cat_to_id.size();
            add_metadata(category);
        }

        m_begin = get_now();
    }

    ~perf()
    {
        auto const now{ get_now()};
        log_duration((now - m_begin).count());
    }
private:

    void prepare_stream(std::fstream& f)
    {
        f.open(file_name);
        f.exceptions(std::ios::badbit | std::ios::failbit);

        f.seekg(-4, std::ios_base::end);
        int const c{ f.peek() };

        f.seekp(-3, std::ios_base::end);
        if (c == '}')
            f << ",\n";
    }

    void add_metadata(std::string const& cat)
    {
        // Format:
        // {"args":{"name":"T1"},"cat":"__metadata","name":"thread_name","ph":"M","pid":1,"tid":1,"ts":0}
        std::fstream f;
        prepare_stream(f);
        f << R"({"args":{"name":")" << cat << "\"";
        f << R"(},"cat":"__metadata","name":"thread_name","ph":"M","pid":0,"tid":)";
        f << m_cat_to_id[cat];
        f << R"(,"ts":0}\n])";
    }

    void log_duration(long long const dur_micro)
    {
        // Format:
        // {"name": "function_name","ph":"X","ts":123,"dur":25319,"pid":0,"tid":2},
        std::fstream f;
        prepare_stream(f);
        f << R"({"name": )" << "\"" << m_func_name << "\"";
        f << R"(,"ph":"X","ts":)" << m_begin.time_since_epoch().count();
        f << R"(,"dur":)" << dur_micro;
        f << R"(,"pid":)" << 0;
        f << R"(,"tid":)" << m_cat_to_id[m_cat];
        f << "}\n]";
    }

    tp m_begin;
    std::string m_func_name;
    std::string m_cat;
    static std::unordered_map<std::string, int> m_cat_to_id;
};

std::unordered_map<std::string, int> perf::m_cat_to_id;

#define PERF_IT(category) perf __p__(__FUNCTION__, category);

namespace ns1
{
    void f1()
    {
        PERF_IT("Task2");
    }
}

int main()
{
    PERF_IT("Task1");
    ns1::f1();
    return 0;
}