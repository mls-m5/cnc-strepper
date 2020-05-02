// Copyright © Mattias Larsson Sköld 2020

#include "command.h"
#include "debug.h"
#include "globals.h"
#include "io.h"
#include "location.h"
#include "testing.h"
#include <cmath>

using namespace std;
using namespace globals;

#define fori(i, o) for (size_t i = 0; i < o.size(); ++i)

namespace {

inline void debugLocation(const Location &l) {
    debugln(l.x);
    debugln(l.y);
    debugln(l.z);
}

struct G1Command : public Command {
    Location target;
    Location from;
    Location direction;
    long operationLength =
        1000000; // Operationlength is the time in microseconds for the
                 // operation to finish (should be called operationDuration
    long maxDimensionLength =
        0;               // The length of the axis that is changing the most
    long realLength = 0; // The real time used to perform the operation
    long progress = 0;
    long maxProgressChange =
        0; // The largest change in progress to do without missing a output step

    long speedAmount = 100;
    long rampLength = 100000;

    std::array<bool, 4> oldValues = {};

    inline void printStepDebugInfo(long progressStep, long dt) const {
        debug("progresstep: ");
        debugln(progressStep);

        debug("dt ");
        debugln(dt);

        debug("progress (abs): ");
        debugln(progress);

        debug("progress %:\t");
        debugln(100 * progress / operationLength);

        debug("\tspeed %:\t");
        debugln(speedAmount);

        fori(i, direction) {
            debug("progresstep: ");
            debugln(progressStep);

            debug("dt ");
            debugln(dt);

            debug("progress (abs): ");
            debugln(progress);

            debug("progress %:\t");
            debugln(100 * progress / operationLength);

            debug("\tspeed %:\t");
            debugln(speedAmount);
        }
    }

    bool operator()(int dt) override {
        using namespace globals;
        realLength += dt;
        if (realLength < rampLength) {
            speedAmount = 100 * realLength / rampLength;
        }
        else {
            speedAmount = 100;
        }

        long progressStep = (long)dt * speedAmount / 100;
        if (progressStep > maxProgressChange) {
            progressStep = maxProgressChange;
            debugln("limiting progress because of processing speed");
        }

        progress += progressStep;

        fori(i, direction) {
            // Without uint64_t this owerflows:
            position[i] =
                from[i] + (int64_t)direction[i] * progress / operationLength;

            int stepValue = abs(position[i] % 2);
            if (stepValue != oldValues[i]) {
                oldValues[i] = stepValue;
                // Perform single step
                digitalWrite(axisPins[i], HIGH);
                digitalWrite(axisPins[i], LOW);
            }
        }

        printStepDebugInfo(progressStep, dt);

        return progress > operationLength;
    }

    Location getTargetFromArguments() {
        return Location{
            .x = getArgumentValueInt('X', position.x, stepsPerMM[0]),
            .y = getArgumentValueInt('Y', position.y, stepsPerMM[1]),
            .z = getArgumentValueInt('Z', position.z, stepsPerMM[2]),
            .e = getArgumentValueInt('E', position.e, stepsPerMM[3]),
        };
    }

    Location getRelativeTargetFromArguments() {
        return Location{
            .x = getArgumentValueInt('X', position.x, stepsPerMM[0]),
            .y = getArgumentValueInt('Y', position.y, stepsPerMM[1]),
            .z = getArgumentValueInt('Z', position.z, stepsPerMM[2]),
            .e = getArgumentValueInt('E', position.e, stepsPerMM[3]),
        };
    }

    void setDirectionPins() {
        fori(i, direction) {
            direction[i] = target[i] - position[i];
            if (directionPins[i] > -1) {
                digitalWrite(directionPins[i], direction[i] > 0);
            }
        }
    }

    // This would be opitimized out if DEBUG is not defined
    void printInitDebugInfo() const {
        debugln("moving to: ");
        debugLocation(target);

        debugLocation(position);
        debugLocation(from);

        debugln("moving from: ");
        debugLocation(from);

        debugln("direction : ");
        debugLocation(direction);

        debug("max progress change: ");
        debugln(maxProgressChange);
    }

    //! Return true on success and when the command should be skipped
    bool init() override {
        using namespace globals;
        debugln("started linear motion");

        if (absolutePositioning) { // Absolute positioning
            target = getTargetFromArguments();
            if (target.x == position.x && target.y == position.y &&
                target.z == position.z && target.e == position.e) {
                return false;
                debugln(F("Already at target position"));
            }
        }
        else {
            target = getRelativeTargetFromArguments();
        }

        fori(i, direction) {
            from[i] = position[i];
        }

        setDirectionPins();

        operationLength = 0;
        fori(i, direction) {
            auto l = abs(direction[i]);
            if (l > maxDimensionLength) {
                maxDimensionLength = l;
                operationLength = maxDimensionLength *
                                  (1000 * 3 / 2); // * 1000 / maxSpeed[i];
            }
        }

        if (!maxDimensionLength) {
            return false;
        }

        maxProgressChange = operationLength / maxDimensionLength;

        printInitDebugInfo();

        return true;
    }
};

} // namespace
std::unique_ptr<Command> createG1Command() {
    return make_unique<G1Command>();
}
