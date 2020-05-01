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
        }

        debug("progresstep: ");
        debugln(progressStep);

        progress += progressStep;
        debug("dt ");
        debugln(dt);

        debug("progress (abs): ");
        debugln(progress);

        debug("progress %:\t");
        debugln(100 * progress / operationLength);

        debug("\tspeed %:\t");
        debugln(speedAmount);
        fori(i, direction) {
            // Without uint64_t this owerflows:
            position[i] =
                from[i] + (int64_t)direction[i] * progress / operationLength;
            if (axisPins[i] > -1) {
                int pinValue = abs(position[i] % 2);
                digitalWrite(axisPins[i], pinValue);
                digitalWrite(led, pinValue);
                debug("axis:\t");
                debug(i);
                debug("\tposition\t");
                debug(position[i]);

                debug("\tpin number\t");
                debug(axisPins[i]);
                debug("\tpinvalue\t");
                debugln(abs(position[i] % 2));
            }
        }

        return progress > operationLength;
    }

    bool init() override {
        using namespace globals;
        debugln("started linear motion");

        if (absolutePositioning) { // Absolute positioning
            target.x = getArgumentValue('X', position.x, stepsPerMM[0]);
            target.y = getArgumentValue('Y', position.y, stepsPerMM[1]);
            target.z = getArgumentValue('Z', position.z, stepsPerMM[2]);
            target.e = getArgumentValue('E', position.e, stepsPerMM[3]);

            if (target.x == position.x && target.y == position.y &&
                target.z == position.z && target.e == position.e) {
                debugln(F("Already at target position"));
            }
        }
        else {
            target.x = position.x + getArgumentValue('X', 0, stepsPerMM[0]);
            target.y = position.y + getArgumentValue('Y', 0, stepsPerMM[1]);
            target.z = position.z + getArgumentValue('Z', 0, stepsPerMM[2]);
            target.e = position.e + getArgumentValue('E', 0, stepsPerMM[3]);
        }

        debugln("moving to: ");
        debugln(target.x);
        debugln(target.y);
        debugln(target.z);
        debugln(target.e);

        fori(i, direction) {
            from[i] = position[i];
            debugln(position[i]);
            debugln(from[i]);
            direction[i] = target[i] - position[i];
            if (directionPins[i] > -1) {
                digitalWrite(directionPins[i], direction[i] > 0);
            }
        }

        debugln("moving from: ");
        debugln(from.x);
        debugln(from.y);
        debugln(from.z);
        debugln(from.e);

        debugln("direction : ");
        debugln(direction.x);
        debugln(direction.y);
        debugln(direction.z);
        debugln(direction.e);

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

        debug("max progress change: ");
        debugln(maxProgressChange);

        return true;
    }
};

} // namespace
std::unique_ptr<Command> createG1Command() {
    return make_unique<G1Command>();
}
