// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include "singlecommand.h"

#include <memory>

std::unique_ptr<SingleCommand> createPositionCommand(PositioningType type);

std::unique_ptr<Command> createG1Command(
    PositioningType PositioningType = PositioningType::Default);

enum class ArcDirection { Clockwise, CounterClockwise };
