// Copyright © Mattias Larsson Sköld 2020

#include "gui.h"

#include "matgui/application.h"
#include "matgui/paint.h"
#include "matgui/progressview.h"
#include "matgui/textentry.h"
#include "matgui/window.h"

#include <iostream>
#include <sstream>

using namespace MatGui;
using namespace std;

namespace {

struct PositionView : View {
    PositionView() {
        height(500);
        linePaint.line.color(1, 1, 1);
    }

    void draw() override {
        auto lx = View::x() + x * 10.;
        auto ly = View::y() + y * 10.;
        linePaint.drawRect(lx - 5, ly - 5, 5, 5);
        linePaint.drawLine(lx, ly, lx + z, ly - z * 5);
    }

    void setValues(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Paint linePaint;

    double x, y, z;
};

} // namespace

struct Gui::Impl {

    Impl() : application(0, nullptr), window("hej") {
        window.addChild(_xView = new ProgressView());
        window.addChild(_yView = new ProgressView());
        window.addChild(_zView = new ProgressView());
        window.addChild(_positionView = new PositionView);
        //        window.addChild(_textEntry = new TextEntry);

        _xView->linear(0, 30);
        _yView->linear(0, 30);
        _zView->linear(0, 10);

        //        cin.rdbuf(cinEmulation.rdbuf());

        //        _textEntry->submit.connect([=]() {
        //            cinEmulation << _textEntry->text() + "\n";
        //            _textEntry->text("");
        //        });
    }

    int run() {
        application.mainLoop();
        return 0;
    }

    void setPosition(double x, double y, double z) {
        if (x != _x || y != _y || z != _z) {
            cout << "gui pos " << x << ", " << y << ", " << z << "\n";
            _x = x;
            _xView->value(x);
            _y = y;
            _yView->value(y);
            _z = z;
            _zView->value(z);

            _positionView->setValues(x, y, z);
        }
    }

private:
    Application application;
    Window window;

    double _x = 0, _y = 0, _z = 0;

    ProgressView *_xView;
    ProgressView *_yView;
    ProgressView *_zView;
    PositionView *_positionView;
    TextEntry *_textEntry;

    //    std::stringstream cinEmulation;
};

Gui::Gui() : _impl(make_unique<Gui::Impl>()) {}

int Gui::run() {
    return _impl->run();
}

void Gui::setPosition(double x, double y, double z) {
    _impl->setPosition(x, y, z);
}

Gui::~Gui() = default;