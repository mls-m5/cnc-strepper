// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include <memory>

class Gui {
public:
    Gui();
    ~Gui();

    void setPosition(double x, double y, double z);

    int run();

    void stop();

private:
    struct Impl;
    std::unique_ptr<struct Impl> _impl;
};
