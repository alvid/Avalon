//
// Created by Aleksey Dorofeev on 06/04/2020.
//

#include "Consumer.hpp"
#include "Store.hpp"

//Потребители должны забрать ВСЕ товары со всех складов. У каждого потребителя только
//одна машина (для упрощения задачи). Так как товары на разных складах могут появляться
//с маленьким промежутком или даже одновременно =&gt; потребитель обзванивает склады и
//пытается забрать товар с первого непустого склада =&gt; этот склад считается просто
//предпочтительным! Но склад не бронирует товар для конкретного потребителя.
//Потребителей одновременно может быть несколько =&gt; &quot;кто первым успел&quot;. Если на
//складе товаров не оказалось, или склад занят приемкой товара от поставщика, или
//обслуживает другого потребителя, потребитель ждет заданный интервал времени. Если
//интервал истек, а склад занят обслуживанием других клиентов или пуст, потребитель
//должен снова обзванивать склады и отправляться на тот склад, где появился товар. Если
//товаров ни на одном из складов не осталось, все потребители должны завершиться.

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
#if __APPLE__
    pthread_setname_np("Consumer__receive_goods");
#endif

    Timeter2 tm;

    std::string good;
    Store::Ret_code rc;
    do {
        rc = Store::release(good, ideal_store, wait_for_store);
        goods.push_back(good);
    } while(rc != Store::eEmpty);

    std::cout << "Consumer has finished";

    return tm.reset();
}

std::ostream& Consumer::show_goods(std::ostream& os)
{
    os << "Consumer: ";
    for(auto const& good: goods)
        os << good << ", ";
    return os;
}
