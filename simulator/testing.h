// Copyright © Mattias Larsson Sköld 2020

#pragma once

#ifdef ARDUINO

#include <Arduino.h>

#else

#define F(x) x

#include "debug.h"

#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using std::cin;
using std::cout;
using std::endl;

class SerialImpl {
public:
    template <class T>
    void print(T value) {
        cout << value;
    }

    template <class T>
    void println(T value) {
        cout << value << "\n";
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
        return !buffer.empty();
    }

    int peek() {
        std::unique_lock<std::mutex> g(iomutex);
        return buffer.front();
    }

    std::queue<char> buffer;

    void pushInput(std::string text) {
        std::unique_lock<std::mutex> g(iomutex);
        for (auto c : text) {
            buffer.push(c);
        }
    }

    void startThread() {
        iothread = std::thread([this]() {
            std::string line;
            while (std::getline(cin, line)) {
                pushInput(line + "\n");
            }
            isRunning = false; // Close the program when stdin ends
        });
    }

    std::thread iothread;

    std::mutex iomutex;

    bool isRunning = true;
};

extern SerialImpl Serial;

unsigned long micros();

void delayMicroseconds(long us);

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

void digitalWrite(int pin, int state);

void pinMode(int pin, int mode);

void setup();
void loop();

#endif
