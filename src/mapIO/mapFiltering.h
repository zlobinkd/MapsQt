#pragma once

#include <vector>
#include <tuple>

#include "src/map/node.h"
#include "src/map/way.h"
#include "src/map/relation.h"

using MapInfoTuple = std::tuple<Nodes, Ways, std::vector<Relation>>;

MapInfoTuple dropUntraversableNodes(const Nodes&, const Ways&, const std::vector<Relation>&);
