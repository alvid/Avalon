#pragma once

#include <iostream>
#include <chrono>

using namespace std::chrono;

struct Timeter {
    std::string name;
    time_point<steady_clock> ts;
    time_point<steady_clock> te;
    bool is_reset;

    Timeter(std::string&& a_name = "") : name(std::move(a_name)), is_reset(false) {
        ts = te = steady_clock::now();
    }
    void reset() {
        te = steady_clock::now();
        auto dt = duration<double>(te - ts);
        microseconds us = duration_cast<microseconds>(dt);
        milliseconds ms = duration_cast<milliseconds>(us % seconds(1));
        seconds ss = duration_cast<seconds>(us % minutes(1));
        us = duration_cast<microseconds>(us % milliseconds(1));
        std::cout << "[" << name << "]: execution has completed at " << ss.count() << "s::" << ms.count() << "ms::" << us.count() << "us" << std::endl;
        ts = te;
        is_reset = true;
    }
    ~Timeter() {
        if (!is_reset)
            reset();
    }
};
