#pragma once

#include "QHash"
#include "QColor"

inline uint qHash(const QColor &key, uint seed = 0) {
    return key.rgba() ^ seed;
}