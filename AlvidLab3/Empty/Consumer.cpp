//
// Created by Aleksey Dorofeev on 06/04/2020.
//

#include "Consumer.hpp"
#include "Store.hpp"

Consumer::Consumer(
        size_t an_ideal_store,
        Dur wait_time)
    : ideal_store(an_ideal_store)
    , wait_for_store(wait_time)
{
}

Fut Consumer::receive_goods()
{
    return std::async(std::launch::async, &Consumer::thr_receive_goods, this);
}

Dur Consumer::thr_receive_goods()
{
    Timeter2 tm;

    std::string good;
    Store::Ret_code rc;
    do {
        rc = Store::release(good, ideal_store, wait_for_store);
        goods.push_back(good);
    } while(rc != Store::eEmpty);

    return tm.reset();
}

std::ostream& Consumer::show_goods(std::ostream& os)
{
    os << "Consumer: ";
    for(auto const& good: goods)
        os << good << ", ";
    return os;
}
