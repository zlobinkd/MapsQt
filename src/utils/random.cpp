#include "random.h"

Random::Random(const std::vector<id_t>& data, size_t seed) : _data(data), _rng(seed) {}

id_t Random::rand() const {
    std::uniform_int_distribution<size_t> dist(0, _data.size() - 1);
    return _data[dist(_rng)];
}
