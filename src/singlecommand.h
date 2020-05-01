// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include "command.h"

struct SingleCommand : Command {
    bool operator()(int dt) override {
        return true;
    }
};
