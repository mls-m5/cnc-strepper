// Copyright © Mattias Larsson Sköld 2020

#include "commandfactory.h"
#include "globals.h"
#include "location.h"
#include "testing.h"
#include <cmath>

using namespace std;

namespace {

//! The ArcCommand uses G1 commands internally
struct ArcCommand : public Command {
    ArcCommand(ArcDirection dir) : _direction(dir) {}

    ~ArcCommand() = default;

    void setTargetPosition() {
        using globals::stepsPerMM;
        _from = _config->getFloatPosition();

        if (_positioningType == PositioningType::Absolute) {
            _target = {
                .x = getArgumentValue('X', _from[0]),
                .y = getArgumentValue('Y', _from[1]),
                .z = getArgumentValue('Z', _from[2]),
            };
        }
        else {

            _target = {
                .x = _from[0] + getArgumentValue('X', 0),
                .y = _from[1] + getArgumentValue('Y', 0),
                .z = _from[2] + getArgumentValue('Z', 0),
            };
        }

        if (_positioningType == PositioningType::Absolute) {
            _pivot = {
                .x = getArgumentValue('I', _from[0]),
                .y = getArgumentValue('J', _from[1]),
                .z = getArgumentValue('K', _from[2]),
            };
        }
        else {
            _pivot = {
                .x = _from[0] + getArgumentValue('I', 0),
                .y = _from[1] + getArgumentValue('J', 0),
                .z = _from[2] + getArgumentValue('K', 0),
            };
        }
    }

    void calculateAngles() {
        auto dx = _from.x - _pivot.x;
        auto dy = _from.y - _pivot.y;
        _radius = sqrt(dx * dx + dy * dy);
        _fromAngle = atan2(dy, dx);
        auto tdx = _target.x - _pivot.x;
        auto tdy = _target.y - _pivot.y;
        _targetAngle = atan2(tdy, tdx);
        _differenceAngle = _targetAngle - _fromAngle;
        if (_direction == ArcDirection::Clockwise) {
            while (_differenceAngle < 0) {
                _differenceAngle += TWO_PI;
            }
        }
        else {
            while (_differenceAngle > 0) {
                _differenceAngle -= TWO_PI;
            }
        }

        _numStep = static_cast<int>(_radius * abs(_differenceAngle));
        _numStep = max(_numStep, 8);
    }

    Status init(globals::Config &config) override {
        _config = &config;

        if (_positioningType == PositioningType::Default) {
            _positioningType = config.positioningType;
        }

        setTargetPosition();

        if (_target == _from) {
            return Status::Finished;
        }

        calculateAngles();

        _linearCommand = generateNextCommand();

        return Running;
    }

    unique_ptr<Command> generateNextCommand() {

        auto fromLocation = _config->getFloatPosition();

        auto stepf = _step;
        auto numStepf = _numStep;

        auto progress = (stepf + 1.) / numStepf;

        auto angle = _fromAngle + _differenceAngle * progress;
        auto x = _pivot.x + _radius * cos(angle);
        auto y = _pivot.y + _radius * sin(angle);
        auto z = _from.z + progress * (_target.z - _from.z);

        auto command = createG1Command(PositioningType::Absolute,
                                       {
                                           {'X', x},
                                           {'Y', y},
                                           {'Z', z},
                                       });

        if (command->init(*_config) == Status::Finished) {
            return {}; // Initialization failed
        }

        return command;
    }

    Status operator()(int dt) override {
        if (!_linearCommand) {
            return Finished;
        }

        if ((*_linearCommand)(dt) == Command::Finished) {
            ++_step;

            if (_step >= _numStep) {
                return Finished;
            }

            //! Create next command
            _linearCommand = generateNextCommand();
            if (!_linearCommand) {
                return Finished;
            }
        }

        return Running;
    }

    Locationf _from;
    Locationf _target;
    Locationf _pivot;
    Locationf _lastFrom;
    float _radius;
    float _fromAngle = 0;
    float _targetAngle = 0;
    float _differenceAngle = 0;
    ArcDirection _direction;

    float _currentAngle = 0;
    int _arcDirection = 0;
    int _numStep = 8;
    int _step = 0;

    std::unique_ptr<Command> _linearCommand;

    globals::Config *_config;

    PositioningType _positioningType = PositioningType::Default;
};

} // namespace

std::unique_ptr<Command> createArcCommand(ArcDirection dir) {
    return make_unique<ArcCommand>(dir);
}
