#include "map.h"

#include "mapData.h"

#include <algorithm>
#include <iostream>

Map::Map()
    : _graphRepresentation([](const Way&){ return true; })
{}

const QHash<QPair<QColor, int>, QVector<QLineF>>& Map::waysToVisualize(const Bounds& bounds) const {
	return _guiRepresentation.waysToVisualize(bounds);
}

std::vector<Connection> Map::shortestPath(id_t from, id_t to) {
	return _graphRepresentation.shortestPath(from, to);
}

id_t Map::closestPoint(const double lat, const double lon, const Bounds& bounds) const {
	return _guiRepresentation.closestPoint(lat, lon, bounds);
}
