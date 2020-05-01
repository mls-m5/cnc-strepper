// Copyright © Mattias Larsson Sköld 2020

#include "commandfactory.h"
#include "debug.h"
#include "globals.h"
#include "singlecommand.h"
#include "testing.h"
#include <memory>

using namespace std;
using namespace globals;

namespace {

struct PositioningCommand : public SingleCommand {
    PositioningCommand(bool positioningType)
        : positioningType(positioningType) {}
    bool init() override {
        arguments.clear();
        debug(F("Setting absolute positioning to "));
        debugln(positioningType);
        absolutePositioning = positioningType;
        return true;
    }

    bool positioningType;
};

} // namespace

unique_ptr<SingleCommand> createPositionCommand(bool type) {
    return make_unique<PositioningCommand>(type);
}
