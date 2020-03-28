// task3.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <random>
#include <thread>
#include <future>
#include <numeric>
#include <cassert>
#include <type_traits>
#include <vector>
#include <deque>

#include "..\..\..\Common\Timeter.hpp"

template <typename CONT>
void printT(CONT const& data_storage)
{
    using Future_type = std::future<typename CONT::value_type>;
    Future_type fut;
    std::cout << "typeid is " << typeid(fut).name() << std::endl;

    std::vector<Future_type> fv;
    fv.push_back(std::async([] {
        std::cout << "sync or async - this is the question" << std::endl;
        return typename CONT::value_type(0);
        }));
}

template <typename CONT>
typename CONT::value_type parallelAccumulate(CONT const& data_storage, int thread_count)
{
    static_assert(std::is_integral_v<typename CONT::value_type>
        || std::is_floating_point_v<typename CONT::value_type>, "Integral of Floating type is required!");

    using Value_type = typename CONT::value_type;
    using Future_type = std::future<Value_type>;

    std::vector<Future_type> partial_sum;
    int elem_range = data_storage.size() / thread_count;
    auto first = data_storage.cbegin();
    auto last = first;
    for (int i = 0; i < thread_count; ++i) {
        last += elem_range;//std::advance(last, elem_range); //непонятно, в чем выигрыш цикла по сравнению с инкрементом итератора?!
        partial_sum.push_back(std::async(std::launch::async, []
            (typename CONT::const_iterator first, typename CONT::const_iterator last) {
                return std::accumulate(first, last, Value_type(0));
            }, first, last));
        first = last;
    }
    partial_sum.push_back(std::async(std::launch::async, []
        (typename CONT::const_iterator first, typename CONT::const_iterator last) {
            return std::accumulate(first, last, Value_type(0));
        }, last, data_storage.cend()));

    Value_type sum(0);
    std::for_each(partial_sum.begin(), partial_sum.end(), [&sum](Future_type& fut) {
        sum += fut.get();
        });

    return sum;
}

int main()
{
    enum {
        DEFAULT_THREAD_COUNT = 2,
        ELEMENTS_COUNT = 100'005,
    };

    int thread_count = std::thread::hardware_concurrency();
    std::cout << "Core(s) detected: " << thread_count << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(INT_MIN, INT_MAX);

    std::vector<int> vec(ELEMENTS_COUNT);
    for (size_t i = 0; i < ELEMENTS_COUNT; ++i)
        vec.push_back((int)dis(gen));

    {
        Timeter tm("sequential accumulate");
        auto sum = std::accumulate(vec.begin(), vec.end(), 0);
        std::cout << "result is " << sum << std::endl;
    }
    {
        Timeter tm("parallel accumulate");

        std::vector<std::future<decltype(vec)::value_type>> partial_sum;
        int elem_range = vec.size() / thread_count;
        auto first = vec.begin();
        auto last = first;
        for (int i = 0; i < thread_count; ++i) {
            last += elem_range;//std::advance(last, elem_range); //непонятно, в чем выигрыш цикла по сравнению с инкрементом итератора?!
            partial_sum.push_back(std::async(std::launch::async, []
            (decltype(vec)::const_iterator first, decltype(vec)::const_iterator last) {
                    return std::accumulate(first, last, 0);
                }, first, last));
            first = last;
        }
        partial_sum.push_back(std::async(std::launch::async, []
        (decltype(vec)::const_iterator first, decltype(vec)::const_iterator last) {
                return std::accumulate(first, last, 0);
            }, last, vec.end()));

        decltype(vec)::value_type sum = 0;
        std::for_each(partial_sum.begin(), partial_sum.end(), [&sum](decltype(partial_sum)::value_type& fut) {
            sum += fut.get();
            });
        std::cout << "result is " << sum << std::endl;
    }

    std::deque<double> dd;
    for (size_t i = 0; i < ELEMENTS_COUNT; ++i)
        dd.push_back(dis(gen));

    {
        Timeter tm("deque: sequential accumulate");
        auto sum = std::accumulate(dd.begin(), dd.end(), 0.0);
        std::cout << "result is " << sum << std::endl;
    }

    {
        Timeter tm("deque: parallel accumulate");
        auto sum = parallelAccumulate(dd, thread_count);
        std::cout << "result is " << sum << std::endl;
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
