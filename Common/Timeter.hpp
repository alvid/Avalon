#pragma once

#include <iostream>
#include <chrono>

using namespace std::chrono;

std::tuple<seconds, milliseconds, microseconds> split_duration(duration<double> const& dt);

struct Timeter {
    std::string name;
    time_point<steady_clock> ts;
    time_point<steady_clock> te;
    bool is_reset;

    Timeter(std::string&& a_name = "") 
        : name(std::move(a_name))
        , is_reset(false)
    {
        ts = te = steady_clock::now();
    }
    void reset() {
        te = steady_clock::now();
        auto dt = duration<double>(te - ts);
        auto [ss, ms, us] = split_duration(dt);
        std::cout << "[" << name << "]: execution has completed at " << ss.count() << "s::" << ms.count() << "ms::" << us.count() << "us" << std::endl;
        ts = te;
        is_reset = true;
    }
    ~Timeter() {
        if (!is_reset)
            reset();
    }
};

struct Timeter2 {
    time_point<steady_clock> ts;
    time_point<steady_clock> te;

    Timeter2()
    {
        ts = te = steady_clock::now();
    }

    duration<double> reset() {
        te = steady_clock::now();
        auto dt = duration<double>(te - ts);
        ts = te;
        return dt;
    }
};

