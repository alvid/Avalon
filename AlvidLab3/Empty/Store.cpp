//
// Created by Aleksey Dorofeev on 06/04/2020.
//

#include <thread>
#include <algorithm>

#include "Store.hpp"
#include "../../Common/Timeter.hpp"

void Store::create(std::vector<size_t> const& all_capacities)
{
    for(auto const& item: all_capacities)
        stores.push_back(std::make_unique<Store>(item));
}

void Store::acquire(std::string const& good, size_t ideal_store, std::chrono::duration<double> dur)
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

Store::Store(size_t a_capacity)
    : capacity(a_capacity)
{
}

bool Store::try_acquire(std::string const& good, std::chrono::duration<double> dur)
{
    if(mt.try_lock_for(dur)) {
        if(goods.size() < capacity) {
            goods.push_front(good);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    if(mt.try_lock_for(dur)) {
        if(!goods.empty()) {
            good = goods.back();
            goods.pop_back();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            mt.unlock();
            return eOk;
        }
        mt.unlock();
        return eEmpty;
    }
    //timeout
    return goods.empty() ? eEmpty : eBusy;
}
