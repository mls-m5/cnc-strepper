// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include "command.h"
#include "location.h"
#include <array>

#include <memory>
#include <queue>

enum class PositioningType {
    Default, // Use whatever is in the config
    Absolute,
    Relative,
};

namespace globals {

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
constexpr int led = 13;

// the pins for the stepper motors
constexpr int STEP = 2;
constexpr int DIR = 3;
constexpr int STEP2 = 4;
constexpr int DIR2 = 5;
constexpr int STEP3 = 6;
constexpr int DIR3 = 7;
extern const std::array<int, 4> axisPins;
extern const std::array<int, 4> directionPins;
extern const std::array<int, 4> stepsPerMM;
extern const std::array<int, 4> maxSpeed;

struct Config {
    PositioningType positioningType;

    Location position;
    unsigned long previousMicros;
    int previousCommand;
    int previousCommandNumber;

    std::queue<std::unique_ptr<class Command>> commands;

    //! Convert stepp-position to float and return it
    Locationf getFloatPosition();
};

extern Config config;

} // namespace globals
