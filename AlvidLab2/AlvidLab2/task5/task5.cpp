// task5.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
//Задание 5. Потокобезопасная кольцевая очередь фиксированного размера на базе условных переменных
//Имитируем формирование и обработку «заявок» посредством многопоточной очереди.
//Потоки - писатели заносят очередную «заявку» в конец очереди (при этом, если в очереди нет места, 
//писатель должен ждать, пока место появится).
//Потоки - читатели должны изъять заявку из начала очереди и обработать (при этом, если в очереди заявок нет, 
//читатель должен ждать, пока какой - нибудь писатель заявку сформирует).

#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <random>
#include <functional>
#include <string>
#include <future>

#include "..\..\..\Common\Timeter.hpp"
#include "..\..\..\Common\Fifo.hpp"

enum {
    FIFO_SIZE = 32,
    WORKER_TIME_NS = 2000,
};

using Task = std::function<int()>;
using Fifo_type = Fifo<Task, FIFO_SIZE>;

// The simple function, that is used as Task for FIFO
template <typename T>
T sum(std::vector<T> const& numbers)
{
    static_assert(std::is_constructible_v<T>);
    return std::accumulate(numbers.begin(), numbers.end(), T(0));
}

// Функция генерирует задачи для исполнителя.
// Размер вектора для задачи расчета задается параметром elem_count.
std::ostringstream producer_routine(Fifo_type& fifo, size_t elem_count)
{
    std::ostringstream oss;
    std::random_device rd;
    std::mt19937 g(rd());

    Timeter2 tm;

    for (;;) {
        std::vector<int> numbers;
        for (size_t j = 0; j < elem_count; ++j)
            numbers.push_back(g());
        auto res = fifo.push_back([numbers = std::move(numbers)] { return sum(numbers); });
        if (res == Fifo_type::Ret_code::eInterrupted)
            break;
    }

    auto work_time = tm.reset();
    auto [ss1, ms1, us1] = split_duration(work_time);
    auto idle_time = fifo.wait_for_free_space;
    auto [ss2, ms2, us2] = split_duration(idle_time);
    oss << "producer[" << std::this_thread::get_id() << "]: " << std::endl
        << "\tpush " << fifo.push_tasks << " tasks" << std::endl
        << "\twork for " << ss1.count() << "s::" << ms1.count() << "ms::" << us1.count() << "us" << std::endl
        << "\twait for " << ss2.count() << "s::" << ms2.count() << "ms::" << us2.count() << "us" << std::endl
        << "\tidle time " << 100 * idle_time / work_time << " %" << std::endl;
    return oss;
}

std::ostringstream worker_routine(Fifo_type& fifo)
{
    std::ostringstream oss;
    Fifo_type::Ret_code res;
    Task task;

    Timeter2 tm;

    while ((res = fifo.pop_front(task)) != Fifo_type::Ret_code::eInterrupted) {
        if (task)
            int rc = task();
        // эмулируем тяжелую работу
        std::this_thread::sleep_for(std::chrono::nanoseconds(WORKER_TIME_NS));
    }

    auto work_time = tm.reset();
    auto [ss1, ms1, us1] = split_duration(work_time);
    auto idle_time = fifo.wait_for_task;
    auto [ss2, ms2, us2] = split_duration(idle_time);
    oss << "worker[" << std::this_thread::get_id() << "]: " << std::endl
        << "\tpop " << fifo.pop_tasks << " tasks" << std::endl
        << "\twork for " << ss1.count() << "s::" << ms1.count() << "ms::" << us1.count() << "us" << std::endl
        << "\twait for " << ss2.count() << "s::" << ms2.count() << "ms::" << us2.count() << "us" << std::endl
        << "\tidle time " << 100 * idle_time / work_time << " %" << std::endl;
    return oss;
}

int main()
{
    enum {
        WORKER_THREAD_COUNT = 2,
        EXECUTOR_THREAD_COUNT = 2,
        ELEM_COUNT = 3'000,
        WORK_SECONDS = 3
    };

    Fifo_type fifo;

    std::vector<std::future<std::ostringstream>> threads;
    threads.reserve(EXECUTOR_THREAD_COUNT + WORKER_THREAD_COUNT);

    for (size_t i = 0; i < WORKER_THREAD_COUNT; ++i)
        threads.push_back(std::async(std::launch::async, &producer_routine, std::ref(fifo), (size_t)ELEM_COUNT));

    for (size_t i = 0; i < EXECUTOR_THREAD_COUNT; ++i)
        threads.push_back(std::async(std::launch::async, &worker_routine, std::ref(fifo)));

    for (int i = 0; i < WORK_SECONDS*10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "fifo state: " << fifo.state() << std::endl;
    }

    std::vector<Task> nft;
    fifo.stop(nft);

    std::cout << std::endl;
    for (auto& item : threads) {
        auto oss = item.get();
        std::cout << oss.str() << std::endl;
    }
    std::cout << "fifo has " << nft.size() << " non finished task(s)" << std::endl;
}

//Результаты для 2-х потоков-писателей и 2-х читателей:
//
//producer[3396]:
//push 441 tasks
//work for 3s::85ms::791us
//wait for 2s::167ms::335us
//idle time 70.236 %
//
//producer[5004]:
//push 2053 tasks
//work for 3s::86ms::2us
//wait for 0s::31ms::14us
//idle time 1.00502 %
//
//worker[16484]:
//pop 1353 tasks
//work for 3s::86ms::143us
//wait for 2s::28ms::697us
//idle time 65.7357 %
//
//worker[22132]:
//pop 1141 tasks
//work for 3s::85ms::235us
//wait for 2s::221ms::474us
//idle time 72.0034 %
//

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
