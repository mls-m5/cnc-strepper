// Copyright © Mattias Larsson Sköld 2020

#include "testing.h"

#include "gui.h"

SerialImpl Serial;

namespace {

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
        if (_tick == val) {
            return;
        }

        _tick = val;

        if (val == 0) {
            return; // Only trigger on rising edge
        }
        _angle += (_direction) ? 1. / 300 : -1. / 300;
        debugln("new angle " + _name + " = " + std::to_string(_angle));
    }

    auto angle() {
        return _angle;
    }
};

StepperMock stepper[4] = {"X", "Y", "Z", "E"};

} // namespace

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

const auto helpText = R"_(
G0 Linear fast move (not implemented)
G1 Linear slow move
G2 Clockwise arc move (X,Y,Z) -> target. (I,J,K) --> Center point
G2 Counter-clockwise arc move  -||-
G90 Absolute positioning
G91 Relative positioning
                      )_";

int main(int /*argc*/, char const ** /*argv*/) {
    using namespace std::chrono_literals;

    cout << helpText << endl;

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
        gui.stop();
    });

    loopThread.detach();

    return gui.run();
}
