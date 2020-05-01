// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include "singlecommand.h"

#include <memory>

std::unique_ptr<SingleCommand> createPositionCommand(bool type);

std::unique_ptr<Command> createG1Command();
