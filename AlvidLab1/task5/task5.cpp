// task5.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <random>
#include <cassert>
#include <atomic>

#include "MtStack.hpp"

///////////////////////////////////////////////////////////////////////////////

static std::once_flag once;
static std::atomic_bool is_producer_started = false;
static std::atomic_bool is_stopped = false;

// поставщик значений для стека
void producer(MtStack<double> &stack, std::chrono::microseconds timeout_us)
{
    size_t count = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1000.0, 1000.0);

    std::call_once(once, [] { is_producer_started = true; });

    while (!is_stopped) {
        stack.push(dis(gen));
        ++count;
        std::this_thread::sleep_for(timeout_us);
    }

    std::cout << "producer[#" << std::this_thread::get_id() << "]: push " << count << " elems" << std::endl;
}

// получатель значений из стека
template <typename T>
void consumer1(MtStack<T>& stack, std::chrono::microseconds timeout_us)
{
    size_t count = 0;
    size_t errors = 0;

    while (!is_stopped) {
        try {
            T elem;
            if (stack.empty())
                continue;
            // в этом месте стек может прочитать другой поток и мы поймаем исключение
            stack.pop(elem);
            ++count;
            std::this_thread::sleep_for(timeout_us);
        }
        catch (std::out_of_range &e) {
            //std::cerr << "consumer1: " << e.what() << std::endl;
            ++errors;
            std::this_thread::sleep_for(timeout_us * 100);
        }
    }
    std::cout << "consumer[#" << std::this_thread::get_id() << "]: pop " << count << " elems" 
        << ", miss " << errors << " times" << std::endl;
}

// получатель значений из стека
template <typename T>
void consumer2(MtStack<T>& stack, std::chrono::microseconds timeout_us)
{
    size_t count = 0;
    size_t errors = 0;

    while (!is_stopped) {
        try {
            if (stack.empty())
                continue;
            // в этом месте стек может прочитать другой поток и мы поймаем исключение
            auto pElem = stack.pop();
            ++count;
            std::this_thread::sleep_for(timeout_us);
        }
        catch (std::out_of_range & e) {
            //std::cerr << "consumer2: " << e.what() << std::endl;
            ++errors;
            std::this_thread::sleep_for(timeout_us * 10);
        }
    }
    std::cout << "consumer[#" << std::this_thread::get_id() << "]: pop " << count << " elems"
        << ", miss " << errors << " times" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    enum {
        PRODUCER_THREAD_COUNT = 3,
        RUN_SECONDS = 10,
    };

    // Тест на корректность конструкторов, операторов копии и перемещения
    {
        MtStack<double> sd1("double1");
        MtStack<double> sd2("double2");
        sd2.push(1.1);
        sd2.push(2.2);
        MtStack<double> sd3("double3");
        sd3.push(3.3);
        sd3.push(4.4);
        sd3.push(5.5);

        sd3 = sd2;
        sd1 = std::move(sd2);
        MtStack<double> sd4(sd1);
    }
    
    // Тест на многопоточное использование стека
    MtStack<double> sd1("double1");
    MtStack<double> sd2("double2");

    std::vector<std::thread> threads;

    for (size_t i = 0; i < PRODUCER_THREAD_COUNT; ++i)
        threads.emplace_back(&producer, std::ref(sd1), std::chrono::microseconds(1000));
    for (size_t i = 0; i < PRODUCER_THREAD_COUNT; ++i)
        threads.emplace_back(&producer, std::ref(sd2), std::chrono::microseconds(1000));
    threads.emplace_back(&consumer1<double>, std::ref(sd1), std::chrono::microseconds(100));
    threads.emplace_back(&consumer2<double>, std::ref(sd2), std::chrono::microseconds(600));

    // Дожидаемся запуска хотя бы одного продюсера
    while (!is_producer_started)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    for (size_t i = 0; i < RUN_SECONDS; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // вывод текущего состояния каждого из стеков
        std::cout << "1th stack: size is " << sd1.size() << std::endl;
        std::cout << "2th stack: size is " << sd2.size() << std::endl;

        // меняем стеки местами
        //MtStack<double> tmp(sd1);
        //sd1 = std::move(sd2);
        //sd2 = std::move(tmp);
    }

    is_stopped = true;

    for (auto& th : threads) {
        if (th.joinable())
            th.join();
    }

    std::cout << "1th stack: size is " << sd1.size() << std::endl;
    std::cout << "2th stack: size is " << sd2.size() << std::endl;

    // Вопрос к преподавателю - очень странно работают таймауты..
    // 1) Я запускаю по три продюсера на стек, каждый продюсер должен записывать ~1000 значений/сек,
    // но по факту записывается всего около 1100, почему???
    // 2) Я запускаю потребителя для первого стека с интервалом 100 мкс, он должен вычерпывать стек досуха,
    // но этого не происходит, почему???
    // 3) Второй потребитель работает с интервалом 600 мкс, но по факту он вычерпывает столько же данных,
    // сколько и первый, почему???
    // Буду признателен за разъяснения.
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
