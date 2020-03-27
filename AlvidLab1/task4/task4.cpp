// task4.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>
#include <thread>
#include <random>
#include <chrono>

using namespace std::chrono;

struct Timeter {
    std::string name;
    time_point<steady_clock> ts;
    time_point<steady_clock> te;

    Timeter(std::string&& a_name = "") : name(std::move(a_name)) {
        ts = te = steady_clock::now();
    }
    void reset() {
        te = steady_clock::now();
        auto dt = duration<double>(te - ts);
        microseconds us = duration_cast<microseconds>(dt);
        milliseconds ms = duration_cast<milliseconds>(us % seconds(1));
        seconds ss = duration_cast<seconds>(us % minutes(1));
        us = duration_cast<microseconds>(us % milliseconds(1));
        std::cout << "[" << name << "]: execution has completed at " << ss.count() << "s::" << ms.count() << "ms::" << us.count() << "us" << std::endl;
        ts = te;
    }
    ~Timeter() {
        if(te != ts)
            reset();
    }
};

// потоковая функция
template <typename InputIt, typename OutputIt>
void partialTransform(InputIt first, InputIt last, OutputIt output)
{
    std::transform(first, last, output, [](typename InputIt::value_type value) -> typename OutputIt::value_type {
            return (typename OutputIt::value_type) abs(value);
        });
}

template <class InputCont, class OutputCont>
void parallelTransform(InputCont const& input, OutputCont& output, size_t thread_count) 
{
    std::vector<std::thread> threads;

    size_t parts = input.size() / thread_count;

    auto first = input.begin();
    for (size_t i = 0; i < thread_count; ++i) {
        auto last = first;
        std::advance(last, parts);
        threads.emplace_back(partialTransform<decltype(first), decltype(output.begin())>, first, last, output.begin() + i*parts);
        first = last;
    }
    if (first != input.end())
        partialTransform(first, input.end(), output.begin() + thread_count*parts);
    
    for (auto& item : threads)
        item.join();
}

int main()
{
    enum {
        ELEMENTS_COUNT = 1000000,
    };

    int thread_count = 2;
    std::cout << "Core(s) detected: " << std::thread::hardware_concurrency() << std::endl;
    //std::cout << "Please, input thread count: ";
    //std::cin >> thread_count;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-32768.0, 32768.0);

    std::vector<short> o1(ELEMENTS_COUNT);

    std::vector<int> i1;
    for (size_t i = 0; i < ELEMENTS_COUNT; ++i)
        i1.push_back((int) dis(gen));
    {
        Timeter tm("single thread");
        partialTransform(i1.begin(), i1.end(), o1.begin());
        std::cout << "output vector size is " << o1.size() << std::endl;
    }

    i1.clear();
    for (size_t i = 0; i < ELEMENTS_COUNT; ++i)
        i1.push_back((int)dis(gen));
    {
        Timeter tm("     parallel");
        parallelTransform(i1, o1, thread_count);
        std::cout << "output vector size is " << o1.size() << std::endl;
    }
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
