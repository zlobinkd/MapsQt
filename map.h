#pragma once

#include "relation.h"
#include "graphRepresentation.h"
#include "guiRepresentation.h"

// class for the backend map representation.
class Map {
public:
	Map() = default;

	// ways from the grid position according to current image boundaries
    const QHash<QPair<QColor, int>, QVector<QLineF>>& waysToVisualize(const Bounds&) const;
	// Dijkstra's algorithm
	std::vector<Connection> shortestPath(id_t from, id_t to);
	// closest map entry for the given arbitrary position
	id_t closestPoint(double lat, double lon, const Bounds&) const;

private:
	GuiRepresentation _guiRepresentation;
	GraphRepresentation _graphRepresentation;
};
