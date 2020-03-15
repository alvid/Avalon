// task5.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <random>
#include <cassert>
#include <condition_variable>

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class MtStack {
    std::vector<T> elems;
    mutable std::mutex mt;

public:
    MtStack();
    MtStack(MtStack const& ref);

    MtStack& operator=(MtStack const& ref);

    bool empty() const;
    size_t size() const;
    void push(T&& elem);
    void pop(T& elem);
    std::unique_ptr<T> pop();
};

template <typename T>
MtStack<T>::MtStack()
{
    std::cout << "stack: created" << std::endl;
}

template <typename T>
MtStack<T>::MtStack(MtStack<T> const& ref)
{
    std::lock_guard lock(ref.mt);
    std::copy(ref.elems.begin(), ref.elems.end(), std::back_inserter(elems));
}

template<typename T>
MtStack<T>& MtStack<T>::operator=(MtStack<T> const& ref)
{
    if (this != *ref) {
        std::lock(ref.mt, mt);
        elems.clear();
        std::copy(ref.elems.begin(), ref.elems.end(), std::back_inserter(elems));
    }
    return *this;
}

template <typename T>
bool MtStack<T>::empty() const
{
    std::lock_guard lock(mt);
    return elems.empty();
}

template <typename T>
size_t MtStack<T>::size() const
{
    std::lock_guard lock(mt);
    return elems.size();
}

template <typename T>
void MtStack<T>::push(T&& elem)
{
    std::lock_guard lock(mt);
    elems.push_back(std::move(elem));
}

template <typename T>
void MtStack<T>::pop(T& elem)
{
    std::lock_guard lock(mt);
    if (elems.empty()) {
        throw std::out_of_range("MtStack::pop(): is empty");
    }
    elem = elems.back();
    elems.pop_back();
}

template <typename T>
std::unique_ptr<T> MtStack<T>::pop()
{
    std::lock_guard lock(mt);
    if (elems.empty()) {
        throw std::out_of_range("MtStack::pop(): is empty");
    }
    auto elem = std::make_unique<T>(elems.back());
    elems.pop_back();
    return elem;
}

///////////////////////////////////////////////////////////////////////////////

std::once_flag producer_started;

void producer(MtStack<double> &stack, std::chrono::milliseconds interval, size_t count, std::condition_variable &cv)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1000.0, 1000.0);

    while (count--) {
        stack.push(dis(gen));
        std::call_once(producer_started, [&cv] { cv.notify_one(); });
        std::this_thread::sleep_for(interval);
    }
    std::cout << "producer[#" << std::this_thread::get_id() << "]: finish" << std::endl;
}

template <typename T>
void consumer1(MtStack<T>& stack, std::chrono::milliseconds interval)
{
    while (!stack.empty()) {
        try {
            T elem;
            stack.pop(elem);
            std::this_thread::sleep_for(interval);
        }
        catch (std::out_of_range &e) {
            std::cerr << "consumer1: sheet happens - " << e.what() << "!" << std::endl;
            break;
        }
    }
    std::cout << "consumer1[#" << std::this_thread::get_id() << "]: finish" << std::endl;
}

template <typename T>
void consumer2(MtStack<T>& stack, std::chrono::milliseconds interval)
{
    while (!stack.empty()) {
        try {
            auto pElem = stack.pop();
            std::this_thread::sleep_for(interval);
        }
        catch (std::out_of_range & e) {
            std::cerr << "consumer2: sheet happens - " << e.what() << "!" << std::endl;
            break;
        }
    }
    std::cout << "consumer2[#" << std::this_thread::get_id() << "]: finish" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    enum {
        THREAD_COUNT = 5
    };
    size_t thread_count = THREAD_COUNT;

    MtStack<double> sd;
    
    std::vector<std::thread> threads;
    std::condition_variable cv;
    std::mutex mt;
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back(&producer, std::ref(sd), std::chrono::milliseconds(1), 2000, std::ref(cv));
    }
    
    {
        std::unique_lock lock(mt);
        cv.wait(lock, [&sd] { return sd.empty(); });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::thread ct1(&consumer1<double>, std::ref(sd), std::chrono::milliseconds(1));
    std::thread ct2(&consumer2<double>, std::ref(sd), std::chrono::milliseconds(1));

    size_t cnt;
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        cnt = sd.size(); 
        std::cout << "stack: size is " << cnt << std::endl;
    } while (cnt);

    for (auto& th : threads) {
        if (th.joinable())
            th.join();
    }

    ct1.join();
    ct2.join();
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
