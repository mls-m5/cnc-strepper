// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include <cstddef>

struct Location {
    long x, y, z, e;

    long &operator[](int index) {
        return (&x)[index];
    }

    long operator[](int index) const {
        return (&x)[index];
    }

    size_t size() const {
        return 4;
    }
};
