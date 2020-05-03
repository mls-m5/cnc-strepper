// Copyright © Mattias Larsson Sköld 2020

#pragma once

#include <cstddef>

template <typename T>
struct LocationT {
    T x, y, z;

    T &operator[](int index) {
        return (&x)[index];
    }

    T operator[](int index) const {
        return (&x)[index];
    }

    //! CCompare positions of the two LocationTs
    bool operator==(const LocationT &other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const LocationT &other) const {
        return !((*this) == other);
    }

    size_t size() const {
        return 3;
    }
};

typedef LocationT<long> Location;
typedef LocationT<float> Locationf;
