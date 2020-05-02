// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include "command.h"

struct SingleCommand : Command {
    Status operator()(int dt) override {
        return Finished;
    }
};
