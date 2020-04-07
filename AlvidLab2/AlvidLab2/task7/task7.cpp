// task7.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
//Задание7. Пул потоков
//Разработаем простой пул потоков, который позволяет :
//•	создавать разумное количество потоков
//•	и многократно их использовать, пока существует пул
//•	при уничтожении самого пула потоки также удаляются.
//Ограничения(упрощения) нашей реализации :
//•	все задания выполняют одну и ту же работу над разными наборами данных
//•	количество «работ» / заданий известно заранее(и превышает количество аппаратных потоков)
//•	как задавать время жизни пула ?
//•	Чтобы можно было хоть как - то попробовать измерить время работы, задания будем добавлять в пул все сразу и централизовано в функции main(в жизни обычно задания формируются разными клиентами в произвольном месте, в любое время и в произвольном порядке)
//•	При выполнении задания хотелось бы исключить вывод на консоль / файл, так как вывод должен выполняться исключительно последовательно(= > придется защищать вывод мьютексом) и он(вывод) будет занимать «львиную долю» времени = > эксперимент не будет «чистым» = > в качестве задания можно использовать создание вектора заданного размера, формирование значений элементов(rand())и какие - то действия, например, вычисление суммы
//•	Интересно было бы «посчитать» сколько заданий выполнил каждый поток

#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <numeric>
#include <random>
#include <array>

#include "ThreadPool.hpp"
#include "../Common/Timeter.hpp"

template <typename T>
class Stupid_task : public Task {
public:
	Stupid_task(size_t cnt, T *pRes) : Task(), count(cnt), pResult(pRes)
	{
		static_assert(std::is_constructible_v<T>);
	}

	void do_work() override 
	{
		std::random_device rd;
		std::mt19937 g(rd());
		numbers.reserve(count);
		for (size_t i = 0; i < count; ++i)
			numbers.push_back(g());
		*pResult = std::accumulate(numbers.begin(), numbers.end(), T(0));
	}

private:
	size_t count;
	std::vector<T> numbers;
	T* pResult;
};

int main()
{
	enum {
		THREAD_COUNT = 0,
		TASK_COUNT = 1000,
		NUMBERS_COUNT = 1000
	};

	std::array<int, TASK_COUNT> results;

	Timeter2 tm;
	
	for (size_t i = 0; i < TASK_COUNT; ++i) {
		//TODO:  приводит к крашу в Thread_pool::thread_routine() на вызове оператора () уже удаленного объекта
		//auto task = std::make_unique<Stupid_task<int>>((size_t) NUMBERS_COUNT, &results[i]);
		auto task = new Stupid_task<int>((size_t)NUMBERS_COUNT, &results[i]);
		(*task)();
		delete task;
	}
	auto [ss1, ms1, us1] = split_duration(tm.reset());
	
	{
		//TODO: если переместить блок вниз (или добавить еще один), это приводит к крашу в Thread_pool::thread_routine() на вызове оператора () уже удаленного объекта
		Thread_pool tp(THREAD_COUNT);
		for (size_t i = 0; i < TASK_COUNT; ++i)
			tp.add_task(new Stupid_task<int>((size_t) NUMBERS_COUNT, &results[i]));
		while (tp.task_count())
			std::this_thread::sleep_for(100us);
	}
	auto [ss2, ms2, us2] = split_duration(tm.reset());
		
	{
		std::vector<std::future<void>> futures;
		futures.reserve(TASK_COUNT);
		for (size_t i = 0; i < TASK_COUNT; ++i)
			futures.push_back(std::async(std::launch::async, [task = std::make_unique<Stupid_task<int>>((size_t) NUMBERS_COUNT, &results[i])] {
					(*task.get())();
				}));

		for (auto& item : futures)
			item.get();
	}
	auto [ss3, ms3, us3] = split_duration(tm.reset());

	std::cout << "created " << Task::create_counter << " tasks" << ", deleted " << Task::delete_counter << " tasks" << std::endl
		<< "\tmain program work time is " << ss1.count() << "s::" << ms1.count() << "ms::" << us1.count() << "us" << std::endl
		<< "\t thread pool work time is " << ss2.count() << "s::" << ms2.count() << "ms::" << us2.count() << "us" << std::endl
		<< "\t      thread work time is " << ss3.count() << "s::" << ms3.count() << "ms::" << us3.count() << "us" << std::endl
		<< std::endl;
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
