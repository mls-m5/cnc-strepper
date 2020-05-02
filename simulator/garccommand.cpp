// Copyright © Mattias Larsson Sköld 2020

#include "commandfactory.h"
#include "globals.h"
#include "location.h"

using namespace std;

namespace {

struct ArcCommand : public Command {
    ArcCommand(ArcDirection dir) : _direction(dir) {}

    ~ArcCommand() = default;

    void setTargetPosition() {
        using globals::stepsPerMM;
        auto &position = _config->position;
        if (_positioningType == PositioningType::Relative) {
            _target = {
                .x = getArgumentValueInt('X', position[0], stepsPerMM[0]),
                .y = getArgumentValueInt('Y', position[1], stepsPerMM[1]),
                .z = getArgumentValueInt('Z', position[2], stepsPerMM[2]),
            };
        }
        else {

            _target = {
                .x = position[0] + getArgumentValueInt('X', 0, stepsPerMM[0]),
                .y = position[1] + getArgumentValueInt('Y', 0, stepsPerMM[1]),
                .z = position[2] + getArgumentValueInt('Z', 0, stepsPerMM[2]),
            };
        }
    }

    Status init(globals::Config &config) override {
        int x = 0, y = 0, z = 0;

        _config = &config;

        if (_positioningType == PositioningType::Default) {
            _positioningType = config.positioningType;
        }

        return Running;
    }

    auto generateNextCommand() {
        auto command = createG1Command(PositioningType::Absolute);
    }

    Status operator()(int dt) override {
        if (!_linearCommand) {
            return Finished;
        }

        if ((*_linearCommand)(dt) == Command::Finished) {
            //! Create next command
        }

        return Running;
    }

    Location _from;
    Location _target;
    Location _pivot;
    Location _lastFrom;
    ArcDirection _direction;

    float _progress = 0;
    float _step = 0;
    float _currentAngle = 0;
    int _arcDirection = 0;

    std::unique_ptr<Command> _linearCommand;

    globals::Config *_config;

    PositioningType _positioningType = PositioningType::Default;
};

} // namespace

std::unique_ptr<Command> createArcCommand(ArcDirection dir) {
    return make_unique<ArcCommand>(dir);
}
