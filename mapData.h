#pragma once

#include "core.h"

#include "node.h"
#include "way.h"
#include "relation.h"
#include "bounds.h"

// Singleton for accessing Nodes and Ways
class MapData {
public:
	static const MapData& instance()
	{
		static MapData instance;
		return instance;
	}

	MapData(const MapData&) = delete;
	MapData& operator=(const MapData&) = delete;
	MapData(MapData&&) = delete;
	MapData& operator=(MapData&&) = delete;

	// node ID == vector index
	const Nodes& nodes() const { return _nodes; }
	// way ID == vector index
	const Ways& ways() const { return _ways; }
	// relation ID == vector index
	const std::vector<Relation>& relations() const { return _relations; }
	// whole map boundaries
	const Bounds& bounds() const { return _bounds; }

private:
	MapData();

	Nodes _nodes;
	Ways _ways;
	std::vector<Relation> _relations;
	Bounds _bounds;
};