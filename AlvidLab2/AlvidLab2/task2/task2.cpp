// task2.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <future>
#include <memory>

// целевой объект
struct A {
    char data[2048];

    A(std::string const& init_str)
    {
        memcpy(data, init_str.data(), std::min(init_str.length(), sizeof(data)));
    }
    
    static size_t creator(std::string name, size_t count) 
    {
        std::cout << "[" << name << "]: started" << std::endl;
        std::string s("Vinni puh i vse-vse-vse");
        std::vector<A> va;
        for (size_t i = 0; i < count; ++i)
            va.emplace_back(s);
        std::cout << "[" << name << "]: finished" << std::endl;
        return va.size();
    }
};

size_t tls_creator(std::string name, size_t count)
{
    std::cout << "[" << name << "]: started" << std::endl;
    std::string s("Moya semya i drugie zveri");
    thread_local std::vector<A> va;
    for (size_t i = 0; i < count; ++i)
        va.emplace_back(s);
    std::cout << "[" << name << "]: finished" << std::endl;
    return va.size();
}

int main()
{
    std::cout << "Hello, parallel World!\n";
    //std::ostringstream oss;

    auto f1 = std::async(std::launch::async, [] (std::string name, size_t count) -> int {
        std::cout << "[" << name << "]: started" << std::endl;
        std::string s("tru-la-la i tra-la-la");
        std::vector<std::unique_ptr<A>> va;
        for (size_t i = 0; i < count; ++i)
            va.push_back(std::make_unique<A>(s));
        std::cout << "[" << name << "]: finished" << std::endl;
        return va.size();
        }, "heap_creator", 12345);

    auto f2 = std::async(std::launch::async, &A::creator, "stack_creator", 54321);
 
    auto f3 = std::async(std::launch::async, tls_creator, "tls_creator", 23232);

    std::cout << "A long-long-long-long title" << std::endl;

    std::cout << "heap_creator: " << 1 << f1.get() << std::endl;
    std::cout << "stack_creator: " << 2 << f2.get() << std::endl;
    std::cout << "tls_creator: " << 3 << f3.get() << std::endl;
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
