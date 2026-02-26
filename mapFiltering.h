#pragma once

#include <vector>
#include <tuple>

#include "node.h"
#include "way.h"
#include "relation.h"

using MapInfoTuple = std::tuple<Nodes, Ways, std::vector<Relation>>;

MapInfoTuple dropUntraversableNodes(const Nodes&, const Ways&, const std::vector<Relation>&);