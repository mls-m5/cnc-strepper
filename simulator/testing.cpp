// Copyright © Mattias Larsson Sköld 2020

#include "testing.h"

#include "gui.h"

SerialImpl Serial;

int main(int /*argc*/, char const ** /*argv*/) {
    using namespace std::chrono_literals;

    Gui gui;

    setup();

    Serial.startThread();

    std::thread loopThread([&]() {
        while (Serial.isRunning) {
            loop();
            std::this_thread::sleep_for(
                1ms); // Prevent program from using 100% cpu
                      // this should make the loop to slow
            //		std::this_thread::sleep_for(10us); // Prevent program
            // from using 100% cpu this should not be any problem

            gui.setPosition(
                stepper[0].angle(), stepper[1].angle(), stepper[2].angle());
        }
    });

    loopThread.detach();

    return gui.run();
}

unsigned long micros() {
    using namespace std::chrono;

    auto now = high_resolution_clock::now();
    auto since_epoch = now.time_since_epoch();
    auto micros = std::chrono::duration_cast<microseconds>(since_epoch);

    return static_cast<unsigned long>(micros.count());
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
