// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include "location.h"
#include <array>

#include <memory>
#include <queue>

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

extern bool absolutePositioning;

extern Location position;
extern const std::array<int, 4> axisPins;
extern const std::array<int, 4> directionPins;
extern const std::array<int, 4> stepsPerMM;
extern const std::array<int, 4> maxSpeed;

extern unsigned long previousMicros;
extern int previousCommand;
extern int previousCommandNumber;

extern std::queue<std::unique_ptr<class Command>> commands;

} // namespace globals
