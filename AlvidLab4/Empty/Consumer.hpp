//
// Created by Aleksey Dorofeev on 06/04/2020.
//

#ifndef EMPTY_CONSUMER_HPP
#define EMPTY_CONSUMER_HPP

#include <vector>

#include "Types.hpp"

class Consumer {
public:
    Consumer(
            size_t ideal_store,         //ближайший склад (индекс)
            Dur wait_time);             //максимальное время ожидания

    virtual ~Consumer() = default;

    Fut receive_goods();
    Dur thr_receive_goods();

    std::ostream& show_goods(std::ostream& os);

private:
    std::vector<std::string> goods;
    size_t ideal_store;
    Dur wait_for_store;

};


#endif //EMPTY_CONSUMER_HPP
