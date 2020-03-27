// task1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <algorithm>
#include <execution>
#include "..\..\..\Common\Timeter.hpp"

int main(int argc, char *argv[])
{
    enum {
        VEC_SIZE = 500'000,
    };

    std::cout << "Hello, parallel World!\n";

    Timeter tm("initialization");

    std::vector<int> vec;
    vec.reserve(VEC_SIZE);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(INT_MIN, INT_MAX);

    for (size_t i=0; i<VEC_SIZE; ++i)
        vec.push_back((int) dis(gen));

    tm.reset();

    std::cout << "vector size is " << vec.size() << std::endl;

    {
        Timeter tm1("sequential increment");
        for (auto& item : vec)
            item += 1;
    }
    {
        Timeter tm1("for_each algorithm");
        std::for_each(vec.begin(), vec.end(), [](int val) { return ++val; });
    }
    {
        Timeter tm1("parallel for_each algorithm");
        std::for_each(std::execution::par, vec.begin(), vec.end(), [](int val) { return ++val; });
    }
    {
        Timeter tm1("parallel for_each algorithm");
        std::for_each(std::execution::par, vec.begin(), vec.end(), [](int val) { return ++val; });
    }
    {
        Timeter tm1("transform algorithm");
        std::transform(vec.begin(), vec.end(), vec.begin(), [](int val) { return ++val; });
    }
    {
        Timeter tm1("parallel transform algorithm");
        std::transform(std::execution::par, vec.begin(), vec.end(), vec.begin(), [](int val) { return ++val; });
    }
    {
        Timeter tm1("parallel transform algorithm");
        std::transform(std::execution::par, vec.begin(), vec.end(), vec.begin(), [](int val) { return ++val; });
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
