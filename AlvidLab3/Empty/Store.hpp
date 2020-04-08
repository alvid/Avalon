//
// Created by Aleksey Dorofeev on 06/04/2020.
//

#ifndef EMPTY_STORE_HPP
#define EMPTY_STORE_HPP

#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "Types.hpp"

class Store {
public:
    enum Ret_code {
        eOk,
        eBusy,
        eEmpty
    };

    static void create(std::vector<size_t> const& all_capacities);
    static void acquire(std::string const& good, size_t ideal_store, Dur dur);
    static Store::Ret_code release(std::string &good, size_t ideal_store, Dur dur);

    explicit Store(size_t a_capacity);

private:
    bool try_acquire(std::string const& good, std::chrono::duration<double> dur);
    Ret_code try_release(std::string &good, std::chrono::duration<double> dur);

private:
    inline static std::vector<std::unique_ptr<Store>> stores{};

private:
    std::deque<std::string> goods;  // склад
    const size_t capacity;          // емкость склада
    std::timed_mutex mt;
    std::condition_variable cv_has_space;
    std::condition_variable cv_has_goods;
};

#endif //EMPTY_STORE_HPP
