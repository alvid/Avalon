// task2.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <map>
#include <functional>
#include <thread>
#include <chrono>
#include <cctype>
#include <string>
#include <atomic>
#include <Windows.h>

const DWORD C = 261;
const DWORD D = 293;
const DWORD E = 329;
const DWORD F = 349;
const DWORD G = 392;
const DWORD A = 440;
const DWORD B = 493;

std::map<char, DWORD> FREQ{ {'C', C}, {'D', D}, {'E', E}, {'F', F}, {'G', G}, {'A', A}, {'B', B} };

const std::string Gamma = "C300;D300;E300;F300;G300;A300;B300";

struct Play_note {
	DWORD freq;
	DWORD duration;
};

bool parseNotes(std::string const& melody, std::vector<Play_note> &result)
{
	const char* ptr = melody.data();
	char ch;
	char lexema[10];
	int ix = 0;
	enum EState { eStart, eDurationDetected, eNoteDetected } state = eStart;
	Play_note cur;

	try {
		while ((ch = *ptr++)) {
			switch (state) {
			case eStart:
				if (std::isalpha(static_cast<unsigned char>(ch)) && std::toupper(ch) <= 'G') {
					cur.freq = FREQ[std::toupper(ch)];
					state = eDurationDetected;
				}
				else {
					return false;
				}
				break;

			case eDurationDetected:
				if (std::isdigit(ch)) {
					lexema[ix++] = ch;
					if (ix == sizeof(lexema))
						throw std::out_of_range("too long duration value");
				}
				else {
					lexema[ix++] = '\0';
					ix = 0;
					cur.duration = std::stoi(lexema);
					state = eNoteDetected;
					--ptr;
				}
				break;

			case eNoteDetected:
				if (ch == ';')
					state = eStart;
				else
					return false;
			}
		}
	}
	catch (std::exception & e) {
		std::cerr << "Error while parsing melody: '" << melody << "' - " << e.what() << std::endl;
		return false;
	}
	return true;
}

std::atomic_int running_threads{ 0 };

void play(std::string const& melody)
{
	std::cout << "playing melody" << std::endl;
	std::vector<Play_note> notes;
	if (!parseNotes(melody, notes)) {
		std::cerr << "Error happens" << std::endl;
		return;
	}
	for (auto const& note : notes) {
		if (Beep(note.freq, note.duration) == 0) {
			std::cerr << "Beep() failed(" << GetLastError() << ")" << std::endl;
			break;
		}
	}
	--running_threads;
}

int main()
{
	std::cout << '\a';

	++running_threads;
	std::thread t(play, Gamma);
	t.detach();

	while(running_threads)
		std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
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
