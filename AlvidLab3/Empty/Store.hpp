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
#include <future>

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
    static void stop();

    explicit Store(size_t a_capacity);

private:
    bool try_acquire(std::string const& good, std::chrono::duration<double> dur);
    Ret_code try_release(std::string &good, std::chrono::duration<double> dur);

    bool size(size_t &sz) const;

    static void th_monitor();

private:
    inline static std::vector<std::unique_ptr<Store>> stores{};

private:
    std::deque<std::string> goods;  // склад
    const size_t capacity;          // емкость склада

    mutable std::timed_mutex mt;

    std::mutex mt_has_space;
    std::condition_variable cv_has_space;

    std::mutex mt_has_goods;
    std::condition_variable cv_has_goods;

    inline static std::atomic_bool stopped_flag{false};
    inline static std::future<void> monitor_fut;
};

#endif //EMPTY_STORE_HPP
