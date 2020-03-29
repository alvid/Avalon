//
// Created by Aleksey Dorofeev on 25/03/2020.
//

#pragma once

#include <iostream>
#include <sstream>
#include <mutex>
#include <thread>
#include <vector>
#include <array>
#include <chrono>
#include <atomic>

// Защищенная в плане многопоточного доступа очередь типа FIFO с предварительным резервированием.
// Организована по типу кольцевого буфера.
// В плане использования предполагается несколько потоков-писателей и несколько потоков-читателей.
template <typename Task, size_t N>
class Fifo {
public:
    enum class Ret_code {
        eGood = 0,
        eInterrupted
    };

    Fifo() : stopped_flag(false), ix_read(0), ix_write(0), cur_size(0)
    {
        memset(buf, '_', N);
        buf[N] = '\0';
    }

    Fifo(Fifo const& ref)
    {
        std::unique_lock lock(ref.mt);

        queue = ref.queue;
        memcpy(buf, ref.buf, N);
        ix_read = ref.ix_read;
        ix_write = ref.ix_write;
        cur_size = ref.cur_size;
        stopped_flag = ref.stopped_flag;
    }

    Fifo& operator=(Fifo const& ref)
    {
        if (this != &ref) {
            std::scoped_lock lock(ref.mt, mt);

            queue = ref.queue;
            memcpy(buf, ref.buf, N);
            ix_read = ref.ix_read;
            ix_write = ref.ix_write;
            cur_size = ref.cur_size;
            stopped_flag = ref.stopped_flag;
        }
        return *this;
    }

    // Команда прекращения работы, вынуждает прервать обслуживание заявок из буфера и пробудить все ждущие потоки.
    void stop(std::vector<Task> &non_performed_tasks)
    {
        stopped_flag = true;
        cv_free.notify_all();
        cv.notify_all();

        std::this_thread::yield(); // TODO: этого недостаточно для вывода из ожидания всех ждущих потоков

        std::unique_lock lock(mt);
        while(cur_size) {
            non_performed_tasks.push_back(queue[ix_read++]);
            if(ix_read == queue.max_size()) 
                ix_read = 0;
            --cur_size;
        }
    }

    // Положить задачу в хвост очереди. Если очередь заполнена, поток будет заблокирован до появления свободного места.
    // Ожидающий поток может быть разблокирован по внешнему сигналу прерывания работы, в этом случае задача не кладется
    // в очередь и возвращается код eInterrupted.
    Ret_code push_back(Task const& request)
    {
        std::unique_lock lock(mt);

        bool wait = cur_size == queue.max_size();
        auto ts = std::chrono::steady_clock::now();
        while(cur_size == queue.max_size()) {
            lock.unlock();
            {
                std::unique_lock lock_free(mt_free);
                cv_free.wait(lock_free, [&] { return cur_size != queue.max_size() || stopped_flag; });
                if (stopped_flag)
                    break;
            }
            lock.lock();
        }
        if(wait) {
            auto te = std::chrono::steady_clock::now();
            auto dt = te - ts;
            wait_for_free_space += dt;
        }

        if(stopped_flag)
            return Ret_code::eInterrupted;

        queue[ix_write] = request;
        buf[ix_write] = 'X';
        if(++ix_write == queue.max_size())
            ix_write = 0;
        ++cur_size;

        ++push_tasks;

        if(cur_size == 1)
            cv.notify_one();

        return Ret_code::eGood;
    }

    // Взять задачу из головы очереди. Если очередь пуста, поток будет заблокирован до появления задачи.
    // Ожидающий поток может быть разблокирован по внешнему сигналу, в этом случае задача не кладется в очередь и
    // возвращается код eInterrupted.
    Ret_code pop_front(Task &result)
    {
        std::unique_lock lock(mt);

        bool wait = cur_size == 0;
        auto ts = std::chrono::steady_clock::now();
        while(cur_size == 0) {
            cv.wait(lock, [&] { return cur_size || stopped_flag; });
            if(stopped_flag)
                return Ret_code::eInterrupted;
        }
        if(wait) {
            auto te = std::chrono::steady_clock::now();
            auto dt = te - ts;
            wait_for_task += dt;
        }

        if(stopped_flag)
            return Ret_code::eInterrupted;

        ++pop_tasks;

        result = queue[ix_read];
        buf[ix_read] = '_';
        if(++ix_read == queue.max_size())
            ix_read = 0;
        if(cur_size-- == queue.max_size())
            cv_free.notify_one();
        return Ret_code::eGood;
    }

    std::string state()
    {
        std::unique_lock lock(mt);
        return std::string((char*)buf);
    }

public:
    inline static thread_local std::chrono::duration<double> wait_for_free_space{0};
    inline static thread_local uint64_t push_tasks{0};

    inline static thread_local std::chrono::duration<double> wait_for_task{0};
    inline static thread_local uint64_t pop_tasks{0};

private:
    std::atomic_bool stopped_flag;

    std::mutex mt;
    std::condition_variable cv;

    std::mutex mt_free;
    std::condition_variable cv_free;

    std::array<Task, N> queue;
    size_t ix_read;
    size_t ix_write;
    size_t cur_size;

    char buf[N+1];    // для индикации занятости
};

