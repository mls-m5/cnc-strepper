
#define F(x) x

#include <iostream>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <string>

using std::cin;
using std::cout;
using std::endl;

class {
public:
	template <class T>
	void print(T value) {
		cout << value;
	}

	template <class T>
	void println(T value) {
		cout << value << endl;
	}

	int parseInt() {
		if (buffer.empty()) {
			return 0;
		}
		char c;
		std::string buf;
		while ((c = peek()) && (c >= '0' && c <= '9')) {
			buf.push_back(read());
		}
		return std::stof(buf);
	}

	float parseFloat() {
		if (buffer.empty()) {
			return 0;
		}
		char c;
		std::string buf;
		while ((c = peek()) && ((c >= '0' && c <= '9') || c == '.')) {
			buf.push_back(read());
		}
		return std::stof(buf);
	}

	int read() {
		std::unique_lock<std::mutex> g(iomutex);
		if (buffer.empty()) {
			return 0;
		}
		char c = buffer.front();
		buffer.pop();
		return c;
	}

	bool available() {
//		std::unique_lock<std::mutex> g(iomutex);
		return !buffer.empty();
	}

	int peek() {
		std::unique_lock<std::mutex> g(iomutex);
		return buffer.front();
	}

	std::queue<char> buffer;

	void startThread() {
		iothread = std::thread([this]() {
			while (cin) {
				std::string line;
				std::getline(cin, line);
				std::unique_lock<std::mutex> g(iomutex);
				for (auto c: line) {
					buffer.push(c);
				}
				buffer.push(10); //Newline
			}
		});
	}

	std::thread iothread;

	std::mutex iomutex;
} Serial;

unsigned long micros() {
	using namespace std::chrono;

	auto now = high_resolution_clock::now();
	auto since_epoch = now.time_since_epoch();
	auto micros = std::chrono::duration_cast<microseconds>(since_epoch);

	return micros.count();
}

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1

void digitalWrite(int pin, int state) {
	if (pin == 2) { // only show one pin
		cout << "pin " << pin << " <- " << state << endl;
	}
}

void pinMode(int pin, int mode) {
	cout << "setting mode for " << pin << " to " << mode << endl;
}

#include "stepper.ino"

int main(int argc, char const *argv[]) {
	using namespace std::chrono_literals;
	setup();

	Serial.startThread();

	while(true) {
		loop();
		std::this_thread::sleep_for(1us);
	}

	return 0;
}
