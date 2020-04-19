#include <iostream>
#include <chrono>
#include <thread>
//#include <atomic>
#include <vector>
#include "../../Common/Spinlock.hpp"

//Задание 2. Реализация spinlock. std::atomic_flag
//1. Создайте класс таким образом, чтобы его можно было использовать в качестве мьютекса,
//но без блокировки потока. Реализуйте методы lock(), unlock() и try_lock()
//При реализации требуется предусмотреть максимально возможную производительность
//=> подумать над заданием соответствующего упорядочения
//2. Проверьте использование разработанного класса в обертках типа lock_guard или
//unique_guard
//3. Для проверки работоспособности замените мьютекс в реализации потокобезопасного
//стека (или очереди) из предыдущего задания на разработанный класс
//4. Оцените (качественно) полученный выигрыш на одном и том же наборе данных при
//использовании обычного мьютекса и разработанного spinlock-а

int g_num = 0;  // protected by g_num_mutex
Spinlock g_num_mutex;

void slow_increment_wolock(int id)
{
    for (int i = 0; i < 100; ++i) {
        //g_num_mutex.lock();
        ++g_num;
        //std::cout << id << ":" << i <<" => " << g_num << '\n';
        //g_num_mutex.unlock();
        //std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

// честно использует lock, чтобы войти в КС
void slow_increment(int id)
{
    for (int i = 0; i < 100; ++i) {
        g_num_mutex.lock();
        ++g_num;
        //std::cout << id << ":" << i <<" => " << g_num << '\n';
        g_num_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

// использует try_lock, самостоятельно организует spin
void slow_increment_2(int id)
{
    for (int i = 0; i < 100; ++i) {
        while(!g_num_mutex.try_lock())
            ; //spin
        ++g_num;
        //std::cout << id << ":" << i <<" => " << g_num << '\n';
        g_num_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

// иногда дает промахи, не захватывая КС
void slow_increment_3(int id)
{
    enum {
        TRY_LOCK_COUNT = 10
    };
    for (int i = 0; i < 100; ++i) {
        bool is_locked = false;
        for (int j = 0; j < TRY_LOCK_COUNT; ++j) {
            is_locked = g_num_mutex.try_lock();
            if (is_locked)
                break;
        }
        if (!is_locked)
            continue;
        ++g_num;
        //std::cout << id << ":" << i <<" => " << g_num << '\n';
        g_num_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

int main() {
    {
        std::vector<std::thread> threads;
        unsigned thread_num = std::thread::hardware_concurrency();
        for (unsigned i = 0; i < thread_num; ++i)
            threads.push_back(std::thread(slow_increment, i));
        for (unsigned i = 0; i < thread_num; ++i)
            threads[i].join();
        // интерпретация результата:
        // на 4-х ядрах => 100*4 = 400
    }
    std::cout << "stage 1: " << g_num << std::endl;

    {
        std::vector<std::thread> threads;
        unsigned thread_num = std::thread::hardware_concurrency();
        for (unsigned i = 0; i < thread_num; ++i)
            threads.push_back(std::thread(slow_increment_wolock, i));
        for (unsigned i = 0; i < thread_num; ++i)
            threads[i].join();
        // интерпретация результата:
        // ввиду потенциальной гонки, результат может отличаться от 400
    }
    std::cout << "stage 2: " << g_num << std::endl;

    {
        std::vector<std::thread> threads;
        unsigned thread_num = std::thread::hardware_concurrency();
        for (unsigned i = 0; i < thread_num; ++i) {
            if (i == 3)
                threads.push_back(std::thread(slow_increment_3, i));
            else if (i == 2)
                threads.push_back(std::thread(slow_increment_2, i));
            else
                threads.push_back(std::thread(slow_increment, i));
        }
        for (unsigned i = 0; i < thread_num; ++i)
            threads[i].join();
        // интерпретация результата:
        // на 4-х ядрах, при использовании в качестве поточной функции slow_increment_3(), результат будет отличаться
        // от 400 на столько, сколько раз функция slow_increment_3() промахнется мимо КС
    }
    std::cout << "stage 3: " << g_num << std::endl;
}
