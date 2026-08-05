#include "src/core.h"

#include <random>
#include <vector>

class Random {
public:
    Random() = delete;
    Random(const std::vector<id_t>& data, size_t seed);

    id_t rand() const;

private:
    std::vector<id_t> _data;
    mutable std::mt19937 _rng;
};
