// task6.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
//Задание6.Thread - safeSingleton
//Создайте класс, реализующий паттерн проектирования Синглтон.
//Какие операции с таким объектом нужно запретить ? и как ?
//
//1 вариант - один объект в рамках многопоточного приложения
//2 вариант – для каждого потока один экземпляр

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

class Singleton {
public:
	static Singleton* instance() {
		// Гарантируется однократное безопасное создание экземпляра статического объекта (синглетон Мейерса)
		static Singleton _instance;
		return &_instance;
	}
	void do_work() {
		std::cout << "[thread " << std::this_thread::get_id() << "]: Singleton::do_work()" << std::endl;
	}

private:
	Singleton() {
		std::cout << "Singleton::ctor(" << this << ") on thread " << std::this_thread::get_id() << std::endl;
	}
	~Singleton() {
		std::cout << "Singleton::dtor(" << this << ")" << std::endl;
	}
};

void make_singleton() {
	auto s = Singleton::instance();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	s->do_work();
}

class Tls_singleton {
public:
	static Tls_singleton* instance() {
		if (_instance == nullptr)
			_instance = new Tls_singleton();
		return _instance;
	}
	static void free() {
		delete _instance;
		_instance = nullptr;
	}
	void do_work() {
		std::cout << "[thread " << std::this_thread::get_id() << "]: Tls_singleton::do_work()" << std::endl;
	}

private:
	Tls_singleton() {
		std::cout << "Tls_singleton::ctor(" << this << ") on thread " << std::this_thread::get_id() << std::endl;
	}
	~Tls_singleton() {
		std::cout << "Tls_singleton::dtor(" << this << ")" << std::endl;
	}

private:
	inline static thread_local Tls_singleton* _instance{ nullptr };
};

void make_tls_singleton() {
	auto s = Tls_singleton::instance();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	s->do_work();

	// повторное использование
	s = Tls_singleton::instance();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	s->do_work();
	Tls_singleton::free();
}

int main()
{
	using namespace std::chrono_literals;

	enum {
		THREAD_COUNT = 5,
	};

	//1 вариант - один объект в рамках многопоточного приложения
	std::vector<std::thread> threads;

	for (int i = 0; i < THREAD_COUNT; ++i)
		threads.emplace_back(&make_singleton);

	std::this_thread::sleep_for(1s);

	for (auto& item : threads)
		item.join();
	threads.clear();

	std::cout << std::endl << std::endl;

	//2 вариант – для каждого потока один экземпляр
	for (int i = 0; i < THREAD_COUNT; ++i)
		threads.emplace_back(&make_tls_singleton);

	std::this_thread::sleep_for(1s);

	for (auto& item : threads)
		item.join();
	threads.clear();
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
