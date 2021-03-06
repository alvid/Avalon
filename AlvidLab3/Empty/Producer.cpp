//
// Created by Aleksey Dorofeev on 31/03/2020.
//

#include <string>
#include <future>

#include "Producer.hpp"
#include "Store.hpp"

//Поставщики должны «развозить» по складам свои товары, пока не пристроят ВСЕ. У
//каждого поставщика машин (потоков) ровно столько, сколько товаров. У каждого
//поставщика есть предпочитаемый им склад (ближайший). Но склад может принимать
//товар от другого поставщика или выдавать товар потребителю. Поэтому поставщик ждет
//заданный интервал времени и, если за это время склад не освободился, едет на
//следующий склад и т.д., пока не пристроит товар.

Producer::Producer(
        std::string const& prefix,
        size_t goods_count,
        size_t an_ideal_store,
        Dur wait_time)
   : ideal_store(an_ideal_store)
   , wait_for_store(wait_time)
{
    for(size_t i=0; i<goods_count;)
        goods.push_back(prefix + "_" + std::to_string(++i));
}

Fut Producer::deliver_goods()
{
    return std::async(std::launch::async, &Producer::p_deliver_goods, this);
}

Dur Producer::p_deliver_goods()
{
    Dur time_wait{0};
    std::vector<Fut> futures;
    for(auto const& good: goods)
        futures.push_back(std::async(std::launch::async, &Producer::deliver_good, this, std::ref(good)));
    for(auto &fut: futures)
        time_wait += fut.get();
    return time_wait;
}

Dur Producer::deliver_good(std::string const& good)
{
    Timeter2 tm;
    Store::acquire(good, ideal_store, wait_for_store);
    return tm.reset();
}
