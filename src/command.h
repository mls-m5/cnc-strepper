// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include <vector>

struct Argument {
    char name;
    bool hasValue = true;
    float value = 0;
};

struct Command {
    char command;
    virtual ~Command() {}

    int number;
    std::vector<Argument> arguments;
    bool initialized = false;

    void parseArguments();

    Argument *getArgument(char name);

    float getArgumentValue(char name, float def);

    float getArgumentValue(char name, float def, float multiplier);

    int getArgumentValueInt(char name, int def, int multiplier);

    void print();

    virtual bool operator()(int dt) = 0;

    //! @returns true if success, returns false if command should be skipped
    virtual bool init() = 0;
};
