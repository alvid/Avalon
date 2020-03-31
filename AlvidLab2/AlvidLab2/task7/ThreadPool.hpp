#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
#include <chrono>

class Task {
public:
	inline static std::atomic_uint create_counter{ 0 };
	inline static std::atomic_uint delete_counter{ 0 };

public:
	Task() {
		++create_counter;
	}
	void operator()() {
		do_work();
	}
	virtual ~Task() {
		++delete_counter;
	}

protected:
	virtual void do_work() = 0;
};

class Thread_pool {
public:
	Thread_pool(size_t thread_count) : stopped_flag(false), recv_task_count(0)
	{
		if (thread_count == 0)
			thread_count = std::thread::hardware_concurrency();
		thread_exec_task_count.reserve(thread_count);
		threads.reserve(thread_count);
		for (size_t i = 0; i < thread_count; ++i) {
			thread_exec_task_count.push_back(0);
			threads.emplace_back(&Thread_pool::thread_routine, this, &thread_exec_task_count[i]);
		}
	}
	// Непонятно, для каких целей требуются конструкторы?!..
	Thread_pool(Thread_pool const& ref) = delete;
	Thread_pool(Thread_pool && ref) = delete;
	
	~Thread_pool()
	{
		if (!stopped_flag)
			stop();
	}

	void add_task(Task *task)
	{
		//if (stopped_flag)
		//	throw std::runtime_error("thread pool is already stopped!");
		++recv_task_count;
		std::unique_lock lock(mt);
		tq.push_back(task);
	}

	size_t task_count()
	{
		std::unique_lock lock(mt);
		return tq.size();
	}

private:
	void stop()
	{
		stopped_flag = true;
		for (auto& item : threads)
			item.join();

		std::cout << "thread pool statistics: " << std::endl;
		std::cout << "\treceived thread count: " << recv_task_count << std::endl;
		for (size_t i = 0; i < thread_exec_task_count.size(); ++i)
			std::cout << "\t[thread #" << i << "]: execute " << thread_exec_task_count[i] << " tasks" << std::endl;
		
		// выполняем ждущие задания
		while(tq.size()) {
			auto &task = tq.front();
			(*task)();
			tq.pop_front();
			delete task;
		}
	}

	void thread_routine(int *exec_task_count) 
	{
		using namespace std::chrono_literals;

		while (!stopped_flag) {
			std::unique_lock lock(mt);
			if (tq.empty()) {
				lock.unlock();
				std::this_thread::sleep_for(100us);
				continue;
			}
			auto &task = tq.front();
			tq.pop_front();
			lock.unlock();

			(*task)();
			delete task;

			++(*exec_task_count);
		}
	}

private:
	std::deque<Task*> tq;
	std::mutex mt;
	std::vector<std::thread> threads;
	std::atomic_bool stopped_flag;
	int recv_task_count;
	std::vector<int> thread_exec_task_count;
};
