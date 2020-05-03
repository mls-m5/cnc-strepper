// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include <vector>

struct Argument {
    Argument() = default;

    Argument(char name) : name(name) {}
    Argument(char name, float value)
        : name(name), hasValue(true), value(value) {}

    char name = 0;
    bool hasValue = false;
    float value = 0;
};

namespace globals {

class Config;

} // namespace globals

struct Command {
    enum Status { Finished, Running };

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

    virtual Status operator()(int dt) = 0;

    //! To be runned once just before starting to execute the command
    virtual Status init(globals::Config &) = 0;
};
