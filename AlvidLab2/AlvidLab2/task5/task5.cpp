// task5.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <random>
#include <functional>

#include "Fifo.hpp"

enum {
    FIFO_SIZE = 32,
    WORKER_THREAD_COUNT = 1,
    EXECUTOR_THREAD_COUNT = 1,
    ELEM_COUNT = 10,
    RUN_COUNT = 32000
};

using Task = std::function<int()>;
using Fifo_type = Fifo<Task, FIFO_SIZE>;

// The simple function, that is used as Task for FIFO
template <typename T>
T sum(std::vector<T> numbers)
{
    static_assert(std::is_constructible_v<T>);
    return std::accumulate(numbers.begin(), numbers.end(), T(0));
}

void worker_routine(Fifo_type& fifo, size_t elem_count, size_t run_count)
{
    std::random_device rd;
    std::mt19937 g(rd());

    auto ts = std::chrono::steady_clock::now();

    for (size_t i = 0; i < run_count; ++i) {
        std::vector<int> numbers;
        for (size_t j = 0; j < elem_count; ++j)
            numbers.push_back(g());
        auto res = fifo.push_back(std::bind(&sum<decltype(numbers)::value_type>, std::move(numbers)));
        if (res == Fifo_type::Ret_code::eInterrupted)
            break;
        // эмулируем задержку до прихода следующей задачи
        std::this_thread::sleep_for(std::chrono::nanoseconds(200));
    }

    auto te = std::chrono::steady_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(te - ts);

    //    oss << std::endl
    //        << "worker[" << std::this_thread::get_id() << "]: "
    //        << "pushes " << fifo.push_tasks << " tasks"
    //        << ", run/wait for " << dt.count() << "/" << fifo.wait_for_free_space_us.count() << " us"
    //        << std::endl;
}

void executor_routine(Fifo_type& fifo)
{
    Fifo_type::Ret_code res;
    Task task;

    auto ts = std::chrono::steady_clock::now();

    while ((res = fifo.pop_front(task)) != Fifo_type::Ret_code::eInterrupted) {
        if (task)
            int rc = task();
        // эмулируем тяжелую работу
        std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
    }

    auto te = std::chrono::steady_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(te - ts);

    //    oss << std::endl
    //        << "executor[" << std::this_thread::get_id() << "]: "
    //        << "popes " << fifo.pop_tasks << " tasks"
    //        << ", run/wait for " << dt.count() << "/" << fifo.wait_for_task_us.count() << " us"
    //        << std::endl;
}

int main()
{
    std::cout << "Hello, asynchronous World!" << std::endl;

    Fifo_type fifo;

    std::vector<std::thread> threads(EXECUTOR_THREAD_COUNT + WORKER_THREAD_COUNT);
    std::vector<std::ostringstream> results(EXECUTOR_THREAD_COUNT + WORKER_THREAD_COUNT);

    for (size_t i = 0; i < EXECUTOR_THREAD_COUNT; ++i)
        threads.emplace_back(&executor_routine, std::ref(fifo));

    for (size_t i = 0; i < WORKER_THREAD_COUNT; ++i)
        threads.emplace_back(&worker_routine, std::ref(fifo), ELEM_COUNT, RUN_COUNT);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::vector<Task> nft;
    fifo.stop(nft);

    std::cout << "fifo has " << nft.size() << " non finished task(s)" << std::endl;
    std::cout << "join all threads" << std::endl;

    for (auto& item : threads)
        item.join();

    for (auto& item : results)
        std::cout << item.str() << std::endl;

    return 0;
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
