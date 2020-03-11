// AlvidLab1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <ctype.h>

using namespace std::chrono;

void thread_func(std::string& in_fn)
{
	//std::cout << "run processing thread: " << std::this_thread::get_id() << " for file: '" << in_fn << "'" << std::endl;

	std::ifstream is(in_fn);
	if (is.bad()) {
		std::cerr << "error when opening file: '" << in_fn << "'" << std::endl;
		return;
	}
	std::string out_fn("UC" + in_fn);
	std::ofstream os(out_fn);
	if (os.bad()) {
		std::cerr << "error when creating file: '" << out_fn << "'" << std::endl;
		return;
	}
	std::transform(
		std::istreambuf_iterator<char>(is),
		std::istreambuf_iterator<char>(),
		std::ostreambuf_iterator<char>(os), [](char ch) {
			return toupper(ch);
		});

	//std::cout << "end processing thread: " << std::this_thread::get_id() << " for file: '" << out_fn << "'" << std::endl;
}

struct Timeter {
	std::string name;
	time_point<steady_clock> ts;
	time_point<steady_clock> te;
	
	Timeter(std::string&& a_name = "") : name(std::move(a_name)) {
		ts = steady_clock::now();
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
		reset();
	}
};

using Task = std::function<void()>;

class Thread_pool {
private:
	size_t max_threads;
	size_t running_threads_count;
	std::vector<std::thread> running_threads;
	std::condition_variable cv;
	std::mutex mt;

public:
	Thread_pool(size_t max_thread_count)
		: max_threads(max_thread_count)
		, running_threads_count(0)
	{
	}
	void callAtThread(Task&& task)
	{
		if (running_threads_count == max_threads) {
			std::unique_lock lock(mt);
			cv.wait(lock);
			--running_threads_count;
		}
		running_threads.emplace_back(&Thread_pool::threadFunc, this, std::move(task));
		++running_threads_count;
	}
	~Thread_pool()
	{
		for (auto& item : running_threads)
			item.join();
	}

protected:
	void threadFunc(Task&& task)
	{
		task();
		std::unique_lock lock(mt);
		cv.notify_one();
	}
};

int main(int argc, char *argv[])
{
	std::vector<std::string> filespec;// = { "1.txt", "2.txt", "3.txt", "4.txt" };

	for (int i = 1; i < argc; ++i) {
		filespec.push_back(argv[i]);
	}

	std::cout << "Running program: " << filespec.size() << std::endl;

	// var1
	{
		Timeter tm("sequential");
		for (auto& item : filespec) {
			std::thread th(thread_func, std::ref(item));
			th.join();
		}
	}

	// var2
	{
		std::vector<std::thread> threads;

		Timeter tm("parallel " + std::to_string(filespec.size()));
		for (auto& item : filespec) {
			threads.emplace_back(thread_func, std::ref(item));
		}
		for (auto& item : threads) {
			item.join();
		}
	}

	//var3
	{
		Thread_pool tp(std::thread::hardware_concurrency());
		Timeter tm("parallel " + std::to_string(std::thread::hardware_concurrency()));
		for (auto& item : filespec) {
			tp.callAtThread(std::bind(thread_func, std::ref(item)));
		}
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
