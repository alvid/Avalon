// Empty.cpp
// Задание: эмуляция системы доставки товаров «производитель-склад-потребитель»

#include <iostream>
#include <vector>
#include <chrono>
#include <memory>

#include "Producer.hpp"
#include "Store.hpp"
#include "Consumer.hpp"
#include "../../Common/Timeter.hpp"

using namespace std::chrono;

int main()
{
    using namespace std::chrono_literals;

    Store::create({300, 2000, 600});

    std::vector<std::unique_ptr<Producer>> producers;
    std::vector<Fut> prod_futs;
    std::vector<std::unique_ptr<Consumer>> consumers;
    std::vector<Fut> cons_futs;

    producers.push_back(std::make_unique<Producer>("T-shirt", 17, 0, 100ms));
    producers.push_back(std::make_unique<Producer>("Pants", 22, 1, 700ms));
    producers.push_back(std::make_unique<Producer>("Cap", 10, 1, 250ms));
    producers.push_back(std::make_unique<Producer>("Shoes", 44, 2, 100ms));
    producers.push_back(std::make_unique<Producer>("Shirt", 33, 0, 70ms));

    consumers.push_back(std::make_unique<Consumer>(2, 300ms));
    consumers.push_back(std::make_unique<Consumer>(1, 150ms));
    consumers.push_back(std::make_unique<Consumer>(0, 50ms));

    for(auto &producer: producers)
        prod_futs.push_back(producer->deliver_goods());

    for(auto &fut: prod_futs) {
        auto [ss, ms, us] = split_duration(fut.get());
        std::cout << "Producer has delivered all its goods by the time "
            << ss.count() << "s::" << ms.count() << "ms::" << us.count() << "us" << " sec" << std::endl;
    }
    for(auto &consumer: consumers)
        cons_futs.push_back(consumer->receive_goods());

    for(auto &fut: cons_futs) {
        auto [ss, ms, us] = split_duration(fut.get());
        std::cout << "Consumer has received all its goods by the time "
                  << ss.count() << "s::" << ms.count() << "ms::" << us.count() << "us" << " sec" << std::endl;
    }
    for(auto &item: consumers)
        item->show_goods(std::cout) << std::endl;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
