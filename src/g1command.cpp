// Copyright © Mattias Larsson Sköld 2020

#include "command.h"
#include "debug.h"
#include "globals.h"
#include "io.h"
#include "location.h"
#include "testing.h"
#include <cmath>

using namespace std;

#define fori(i, o) for (size_t i = 0; i < o.size(); ++i)

namespace {

inline void debugLocation(const Location &l) {
    debugln(l.x);
    debugln(l.y);
    debugln(l.z);
}

struct G1Command : public Command {
    Location _target;
    Location _from;
    Location _direction;
    long _operationLength =
        1000000; // Operationlength is the time in microseconds for the
                 // operation to finish (should be called operationDuration
    long _maxDimensionLength =
        0;                // The length of the axis that is changing the most
    long _realLength = 0; // The real time used to perform the operation
    long _progress = 0;
    long _maxProgressChange =
        0; // The largest change in progress to do without missing a output step

    long _speedAmount = 100;
    long _rampLength = 100000;

    PositioningType _positioningType = PositioningType::Default;

    std::array<bool, 4> oldValues = {};

    globals::Config *_config = nullptr;

public:
    G1Command(PositioningType positioningType, std::vector<Argument> arguments)
        : _positioningType(positioningType) {
        this->arguments = move(arguments);
    }

    inline void printStepDebugInfo(long progressStep, long dt) const {
        debug("progresstep: ");
        debugln(progressStep);

        debug("dt ");
        debugln(dt);

        debug("progress (abs): ");
        debugln(_progress);

        debug("progress %:\t");
        debugln(100 * _progress / _operationLength);

        debug("\tspeed %:\t");
        debugln(_speedAmount);

        fori(i, _direction) {
            debug("progresstep: ");
            debugln(progressStep);

            debug("dt ");
            debugln(dt);

            debug("progress (abs): ");
            debugln(_progress);

            debug("progress %:\t");
            debugln(100 * _progress / _operationLength);

            debug("\tspeed %:\t");
            debugln(_speedAmount);
        }
    }

    Status operator()(int dt) override {
        using globals::axisPins;
        _realLength += dt;
        if (_realLength < _rampLength) {
            _speedAmount = 100 * _realLength / _rampLength;
        }
        else {
            _speedAmount = 100;
        }

        long progressStep = (long)dt * _speedAmount / 100;
        if (progressStep > _maxProgressChange) {
            progressStep = _maxProgressChange;
            debugln("limiting progress because of processing speed");
        }

        _progress += progressStep;

        auto &position = _config->position;

        fori(i, _direction) {
            // Without uint64_t this owerflows:
            position[i] = _from[i] +
                          (int64_t)_direction[i] * _progress / _operationLength;

            int stepValue = abs(position[i] % 2);
            if (stepValue != oldValues[i]) {
                oldValues[i] = stepValue;
                // Perform single step
                digitalWrite(axisPins[i], HIGH);
                digitalWrite(axisPins[i], LOW);
            }
        }

        printStepDebugInfo(progressStep, dt);

        return (_progress > _operationLength) ? Finished : Running;
    }

    Location getTargetFromArguments() {
        using globals::stepsPerMM;
        const auto &position = _config->position;
        return {
            .x = getArgumentValueInt('X', position.x, stepsPerMM[0]),
            .y = getArgumentValueInt('Y', position.y, stepsPerMM[1]),
            .z = getArgumentValueInt('Z', position.z, stepsPerMM[2]),
        };
    }

    Location getRelativeTargetFromArguments() {
        using globals::stepsPerMM;
        const auto &position = _config->position;
        return {
            .x = position.x + getArgumentValueInt('X', 0, stepsPerMM[0]),
            .y = position.y + getArgumentValueInt('Y', 0, stepsPerMM[1]),
            .z = position.z + getArgumentValueInt('Z', 0, stepsPerMM[2]),
        };
    }

    void setDirectionPins() {
        using globals::directionPins;
        const auto &position = _config->position;
        fori(i, _direction) {
            _direction[i] = _target[i] - position[i];
            if (directionPins[i] > -1) {
                digitalWrite(directionPins[i], _direction[i] > 0);
            }
        }
    }

    // This would be opitimized out if DEBUG is not defined
    void printInitDebugInfo() const {
        debugln("moving to: ");
        debugLocation(_target);

        const auto &position = _config->position;
        debugLocation(position);
        debugLocation(_from);

        debugln("moving from: ");
        debugLocation(_from);

        debugln("direction : ");
        debugLocation(_direction);

        debug("max progress change: ");
        debugln(_maxProgressChange);
    }

    Status init(globals::Config &config) override {
        this->_config = &config;
        const auto &position = config.position;

        debugln("started linear motion");

        if (_positioningType == PositioningType::Default) {
            _positioningType = config.positioningType;
        }

        if (_positioningType == PositioningType::Absolute) {
            _target = getTargetFromArguments();
            if (_target.x == position.x && _target.y == position.y &&
                _target.z == position.z) {
                return Finished;
                debugln(F("Already at target position"));
            }
        }
        else {
            _target = getRelativeTargetFromArguments();
        }

        fori(i, _direction) {
            _from[i] = position[i];
        }

        setDirectionPins();

        _operationLength = 0;
        fori(i, _direction) {
            auto l = abs(_direction[i]);
            if (l > _maxDimensionLength) {
                _maxDimensionLength = l;
                _operationLength = _maxDimensionLength *
                                   (1000 * 3 / 2); // * 1000 / maxSpeed[i];
            }
        }

        if (!_maxDimensionLength) {
            return Finished;
        }
        if (!_operationLength) {
            return Finished;
        }

        _maxProgressChange = _operationLength / _maxDimensionLength;

        printInitDebugInfo();

        return Running;
    }
};

} // namespace
std::unique_ptr<Command> createG1Command(PositioningType positioningType,
                                         std::vector<Argument> arguments) {
    return make_unique<G1Command>(positioningType, std::move(arguments));
}
