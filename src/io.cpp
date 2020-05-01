// Copyright © Mattias Larsson Sköld 2020

#include "io.h"
#include "debug.h"
#include "testing.h"
#include <cmath>
#include <limits>

float readFloat() {
    auto p = Serial.peek();
    if ((p >= '0' && p <= '9') || p == '.') {
        return Serial.parseFloat();
    }
    else if (p == '-') {
        Serial.read(); // Throw away the - char
        return -Serial.parseFloat();
    }
    else {
        return std::numeric_limits<float>::quiet_NaN();
    }
}

void printChar(char c) {
    char output[2] = {(char)c, (char)0};
    debug(output);
}
