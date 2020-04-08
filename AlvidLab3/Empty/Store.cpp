//
// Created by Aleksey Dorofeev on 06/04/2020.
//

#include <thread>
#include <algorithm>
#include <chrono>
#include <sstream>

#include "Store.hpp"
#include "../../Common/Timeter.hpp"

using namespace std::chrono_literals;

void Store::acquire(std::string const& good, size_t ideal_store, Dur dur)
{
    bool is_good_placed;
    do {
        Store *store = stores[ideal_store].get();
        is_good_placed = store->try_acquire(good, dur);
        if(++ideal_store == stores.size())
            ideal_store = 0;
    } while(!is_good_placed);
}

Store::Ret_code Store::release(std::string &good, size_t ideal_store, Dur dur)
{
    std::vector<Store::Ret_code> results(stores.size(), eBusy);
    Store::Ret_code res = eBusy;
    do {
        Store *store = stores[ideal_store].get();
        res = store->try_release(good, dur);
        if(res == eOk)
            return eOk;
        results[ideal_store] = res;
        if(++ideal_store == stores.size())
            ideal_store = 0;
        std::for_each(results.begin(), results.end(), [&res](Store::Ret_code store_res) {
            if(store_res != eEmpty) res = store_res; });
    } while(res == eBusy);
    return eEmpty;
}

void Store::create(std::vector<size_t> const& all_capacities)
{
    for(auto const& item: all_capacities)
        stores.push_back(std::make_unique<Store>(item));

    monitor_fut = std::async(std::launch::async, &Store::th_monitor);
}

void Store::stop()
{
    stopped_flag = true;
    monitor_fut.get();
}

void Store::th_monitor()
{
#if __APPLE__
    pthread_setname_np("Store__th_monitor");
#endif
    size_t sz;
    while(!stopped_flag) {
        std::ostringstream oss;
        std::this_thread::sleep_for(500ms);
        for (auto const &store: stores) {
            if (store->size(sz))
                oss << "Store: goods count = " << sz << std::endl;
            else
                oss << "Store: busy " << std::endl;
        }
        std::cout << oss.str() << std::endl;
    }
}

Store::Store(size_t a_capacity)
    : capacity(a_capacity)
{
}

bool Store::size(size_t &sz) const
{
    if(mt.try_lock_for(500ms)) {
        sz = goods.size();
        mt.unlock();
        return true;
    }
    return false;
}

bool Store::try_acquire(std::string const& good, std::chrono::duration<double> dur)
{
    Timeter2 tm;
    if(mt.try_lock_for(dur)) {
        // ждем освобождения места на складе
        if(goods.size() == capacity) {
            mt.unlock();
            std::unique_lock lock(mt_has_space);
            dur -= tm.reset();
            if(dur.count() < 0)
                return false;
            cv_has_space.wait_for(lock, dur);
            dur -= tm.reset();
            if(dur.count() < 0)
                return false;
            if(!mt.try_lock_for(dur))
                return false;
        }
        if(goods.size() < capacity) {
            std::this_thread::sleep_for(30ms);
            goods.push_front(good);
            if(goods.size() == 1)
                cv_has_goods.notify_one();
            mt.unlock();
            return true;
        }
        mt.unlock();
    }
    //timeout
    return false;
}

Store::Ret_code Store::try_release(std::string &good, std::chrono::duration<double> dur)
{
    //Timeter2 tm;
    if(mt.try_lock_for(dur)) {
        // ждем поступления товара на склад
        if(goods.empty()) {
            mt.unlock();
            std::unique_lock lock(mt_has_goods);
            //dur -= tm.reset();
            //if(dur.count() < 0)
            //    return eBusy;
            cv_has_goods.wait_for(lock, dur);
            //dur -= tm.reset();
            //if(dur.count() < 0)
            //    return eBusy;
            if(!mt.try_lock_for(dur))
                return eBusy;
        }
        if(!goods.empty()) {
            std::this_thread::sleep_for(25ms);
            good = goods.back();
            goods.pop_back();
            if(goods.size() == capacity-1)
                cv_has_space.notify_one();
            mt.unlock();
            return eOk;
        }
        mt.unlock();
        return eEmpty;
    }
    //timeout
    return goods.empty() ? eEmpty : eBusy;
}
