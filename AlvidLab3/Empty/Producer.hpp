//
// Created by Aleksey Dorofeev on 31/03/2020.
//

#ifndef EMPTY_PRODUCER_HPP
#define EMPTY_PRODUCER_HPP

#include <cstddef>
#include <vector>
#include <chrono>
#include <thread>
#include <future>

#include "Types.hpp"

//Поставщики должны «развозить» по складам свои товары, пока не пристроят ВСЕ. У
//каждого поставщика машин (потоков) ровно столько, сколько товаров. У каждого
//поставщика есть предпочитаемый им склад (ближайший). Но склад может принимать
//товар от другого поставщика или выдавать товар потребителю. Поэтому поставщик ждет
//заданный интервал времени и, если за это время склад не освободился, едет на
//следующий склад и т.д., пока не пристроит товар.

class Producer {
public:
    Producer(
            std::string const& prefix,  //имя поставщика/префикс товара
            size_t goods_count,         //количество товаров
            size_t ideal_store,         //ближайший склад (индекс)
            Dur wait_time);             //максимальное время ожидания

    virtual ~Producer() = default;

    Fut deliver_goods();
    Dur deliver_good(std::string const& good);

private:
    Dur p_deliver_goods();

private:
    std::vector<std::string> goods;
    size_t ideal_store;
    Dur wait_for_store;
};


#endif //EMPTY_PRODUCER_HPP
