// Copyright © Mattias Larsson Sköld 2020

#include "testing.h"
#include "../src/mill.cpp"

#include "gui.h"

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
