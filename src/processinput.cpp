// Copyright © Mattias Larsson Sköld 2020

#include "processinput.h"
#include "command.h"
#include "commandfactory.h"
#include "debug.h"
#include "globals.h"
#include "io.h"
#include "testing.h"
#include <cmath>

using namespace std;
using namespace globals;

namespace {

unique_ptr<Command> createCommand(int c, int number) {
    unique_ptr<Command> command;

    switch (c) {
    case 'G':
        switch (number) {
        case 1:
            return createG1Command(PositioningType::Default);
        case 90:
            return createPositionCommand(PositioningType::Absolute);
        case 91:
            return createPositionCommand(PositioningType::Relative);
        }
        break;
    }

    return {};
}

void processCommand(int c) {
    int number;

    Argument argument; // If this is is shorthand notation, values is
    // stored here
    argument.name = 0;

    c = toupper(c);

    if ((c >= 'X' && c <= 'Z') || (c >= 'I' && c <= 'K')) {
        // When only arguments is given the last argument is reused
        argument.name = c;
        argument.value = readFloat();
        if (std::isnan(argument.value)) {
            argument.hasValue = false;
        }

        c = config.previousCommand;
        number = config.previousCommandNumber;
        debugln("reusing previous command ");

        debugln(config.previousCommandNumber);
    }
    else {
        config.previousCommand = c;
        config.previousCommandNumber = number = Serial.parseInt();
    }

    auto command = createCommand(c, number);

    if (command) {
        command->command = c;
        command->number = number;

        if (argument.name != 0) {
            command->arguments.push_back(argument);
        }
        // Read the rest of the arguments
        command->parseArguments();

        command->print();
        config.commands.push(move(command));
    }
}

void processNext() {
    // read the incoming byte:
    int c = toupper(Serial.read());

    if (c == ';') {
        while (c != 10 && Serial.available() > 0) {
            c = Serial.read();
        }
        debugln("removed comment");
    }
    else if (c == '(') {
        while (c != ')' && Serial.available() > 0) {
            c = Serial.read();
        }
        debugln("removed comment");
    }
    else if ((c >= 'A' && c <= 'Z') || c == ' ') {
        processCommand(c);
    }
}

} // namespace

void processInput() {
    using namespace globals;
    while (Serial.available() > 0) {
        processNext();
    }
}
