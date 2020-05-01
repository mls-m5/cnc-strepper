// Copyright © Mattias Larsson Sköld 2020

#pragma once

#define F(x) x

#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

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
        return std::stoi(buf);
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
                for (auto c : line) {
                    buffer.push(c);
                }
                buffer.push(10); // Newline
            }
            isRunning = false; // Close the program when stdin ends
        });
    }

    std::thread iothread;

    std::mutex iomutex;

    bool isRunning = true;
} Serial;

unsigned long micros() {
    using namespace std::chrono;

    auto now = high_resolution_clock::now();
    auto since_epoch = now.time_since_epoch();
    auto micros = std::chrono::duration_cast<microseconds>(since_epoch);

    return static_cast<unsigned long>(micros.count());
}

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1

class StepperMock {
    std::string _name;
    float _angle = 0;
    bool _direction = 1;
    bool _tick = 0;

public:
    StepperMock(const char *name) : _name(name) {}

    void setDir(bool val) {
        _direction = val;
    }

    void setPin(bool val) {
        if (_tick != val) {
            _angle += (_direction) ? 1. / 300 : -1. / 300;
            cout << "new angle " << _name << " = " << _angle << endl;
        }
        _tick = val;
    }

    auto angle() {
        return _angle;
    }
};

namespace {

StepperMock stepper[4] = {"X", "Y", "Z", "E"};
}

void digitalWrite(int pin, int state) {
    //	if (pin == 2) { // only show one pin
    cout << "pin " << pin << " <- " << state << endl;
    //	}
    if (pin == 2) {
        stepper[0].setPin(state);
    }
    else if (pin == 3) {
        stepper[0].setDir(state);
    }

    if (pin == 4) {
        stepper[1].setPin(state);
    }
    else if (pin == 5) {
        stepper[1].setDir(state);
    }

    if (pin == 6) {
        stepper[2].setPin(state);
    }
    else if (pin == 7) {
        stepper[2].setDir(state);
    }
}

void pinMode(int pin, int mode) {
    cout << "setting mode for " << pin << " to " << mode << endl;
}