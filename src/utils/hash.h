#pragma once

#include "QHash"
#include "QColor"

inline uint qHash(const QColor &key, uint seed = 0) {
    return qHash(key.rgba(), seed);
}
