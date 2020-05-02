// Copyright © Mattias Larsson Sköld 2020

#include "commandfactory.h"
#include "debug.h"
#include "globals.h"
#include "singlecommand.h"
#include "testing.h"
#include <memory>

using namespace std;

namespace {

class PositioningCommand : public SingleCommand {
    PositioningType _positioningType;

public:
    PositioningCommand(PositioningType positioningType)
        : _positioningType(positioningType) {}

    Status init(globals::Config &config) override {
        arguments.clear();
        debug(F("Setting absolute positioning to "));
        debugln(_positioningType == PositioningType::Absolute ? "absolute"
                                                              : "relative");
        config.positioningType = _positioningType;
        return Finished;
    }
};

} // namespace

unique_ptr<SingleCommand> createPositionCommand(PositioningType type) {
    return make_unique<PositioningCommand>(type);
}
