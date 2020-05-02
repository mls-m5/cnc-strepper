// Copyright © Mattias Larsson Sköld 2020

#include "command.h"
#include "debug.h"
#include "globals.h"
#include "io.h"
#include "testing.h"
#include <cmath>

using namespace globals;

void Command::parseArguments() {
    char c;
    while (c != 10) {
        while (Serial.available() <= 0) {
            // Wait for input
        }
        c = Serial.read();
        c = toupper(c);
        if (c >= 'A' && c <= 'Z') {
            Argument argument;
            argument.name = c;
            auto value = readFloat();
            if (std::isnan(value)) {
                argument.hasValue = false;
            }
            else {
                argument.value = value;
            }
            arguments.push_back(argument);
        }
    }
}

void Command::print() {
    printChar(command);
    debug(number);

    debug(" ");

    for (auto arg : arguments) {
        printChar(arg.name);
        debug(arg.value);
        debug(" ");
    }

    switch (command) {
    case 'G':
        switch (number) {
        case 1:
            debugln("linear motion");
            //            switch (positioningType) {
            //            case PositioningType::Absolute:
            //                debug(" absolute positioning");
            //                break;
            //            case PositioningType::Relative:
            //                debug(" relative positioning");
            //                break;
            //            }
            break;
        case 2:
        case 3:
            debugln("arc movement");
            break;
        }
        break;
    }

    debugln("");
}

float Command::getArgumentValue(char name, float def, float multiplier) {
    auto arg = getArgument(name);
    if (arg) {
        return arg->value * multiplier;
    }
    else {
        return def;
    }
}

int Command::getArgumentValueInt(char name, int def, int multiplier) {
    auto arg = getArgument(name);
    if (arg) {
        return multiplier * arg->value;
    }
    else {
        return def;
    }
}

float Command::getArgumentValue(char name, float def) {
    auto arg = getArgument(name);
    if (arg) {
        return arg->value;
    }
    else {
        return def;
    }
}

Argument *Command::getArgument(char name) {
    for (auto &arg : arguments) {
        if (arg.name == name) {
            return &arg;
        }
    }
    return nullptr;
}
