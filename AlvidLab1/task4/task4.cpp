// task4.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>
#include <thread>

// потоковая функция
template <typename InputIt, typename OutputIt>
void partialTransform(InputIt first, InputIt last, OutputIt output)
{
    std::transform(first, last, output, [](typename InputIt::value_type value) -> typename OutputIt::value_type {
            return (typename OutputIt::value_type) abs(value);
        });
}

template <class InputCont, class OutputCont>
void parallelTransform(InputCont const& input, OutputCont& output, size_t thread_count) {
    std::vector<std::thread> threads;
    std::vector<OutputCont> result;

    size_t parts = input.size() / thread_count;
    result.reserve(parts+1);

    auto first = input.begin();
    for (size_t i = 0; i < thread_count; ++i) {
        auto last = first;
        std::advance(last, parts);
        //auto lyambda = [=]() {
            //partialTransform(first, last, result[i]);
        //};
        //threads.emplace_back(lyambda);
        first = last;
    }
//    if (first != input.end())
//        partialTransform(first, input.end(), result[parts]);
    
    for (auto& item : threads)
        item.join();

    for (auto const& item : result)
        for (auto const& item2 : item)
            output.push_back(item2);
}

int main()
{
    int thread_count = 1;
    std::cout << "Core detected: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Please, input thread count: ";
    std::cin >> thread_count;

    std::vector<int> i1{ 1,2,512,-1000,777,-356,-9,0,11,-1122344 };
    std::vector<short> o1;

    parallelTransform(i1, o1, thread_count);
    std::transform(i1.begin(), i1.end(), o1.begin(), [](int value) -> short {
        return (short) abs(value);
        });

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
