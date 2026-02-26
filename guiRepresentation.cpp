#include "guiRepresentation.h"

#include "mapData.h"

#include <limits>

static QPair<QColor, int> routePen(std::string routeName) {
    if (routeName == "footway" || routeName == "path" || routeName == "pedestrian"
        || routeName == "steps" || routeName == "elevator" || routeName == "platform"
        || routeName == "bridleway") {
        return QPair<QColor, int>(Qt::darkGreen, 1);
    }
    else if (routeName == "motorway" || routeName == "primary") {
        return QPair<QColor, int>{Qt::yellow, 2};
    }
    else if (routeName == "motorway_link" || routeName == "primary_link") {
        return QPair<QColor, int>(Qt::darkYellow, 1);
    }
    else if (routeName == "unclassified" || routeName == "tertiary" || routeName == "secondary"
             || routeName == "secondary_link" || routeName == "tertiary_link" || routeName == "residential") {
        return QPair<QColor, int>(Qt::darkGray, 1);
    }
    else if (routeName == "track" || routeName == "construction" || routeName == "service") {
        return QPair<QColor, int>(Qt::magenta, 1);
    }
    else if (routeName == "cycleway") {
        return QPair<QColor, int>(Qt::green, 1);
    }
    else if (routeName == "trunk") {
        return QPair<QColor, int>(Qt::blue, 2);
    }
    else if (routeName == "trunk_link") {
        return QPair<QColor, int>(Qt::cyan, 1);
    }
    else if (routeName == "living_street") {
        return QPair<QColor, int>(Qt::gray, 1);
    }
    return QPair<QColor, int>(Qt::darkGray, 1);
}

static bool skipType(const std::string& routeName) {
    if (routeName == "" || routeName == "cycleway"
        || routeName == "footway" || routeName == "service"
        || routeName == "pedestrian" || routeName == "track"
        || routeName == "path" || routeName == "steps"
        || routeName == "elevator" || routeName == "platform"
        || routeName == "rest_area" || routeName == "raceway"
        || routeName == "corridor" || routeName == "bridleway")
    {
        return true;
    }

    return false;
}

static bool isRoad(std::string routeName) {
	if (routeName == "footway" || routeName == "path" || routeName == "pedestrian"
		|| routeName == "steps" || routeName == "elevator" || routeName == "platform"
		|| routeName == "bridleway") {
		return false;
	}
	else if (routeName == "motorway" || routeName == "primary") {
		return true;
	}
	else if (routeName == "motorway_link" || routeName == "primary_link") {
		return true;
	}
	else if (routeName == "unclassified" || routeName == "tertiary" || routeName == "secondary"
		|| routeName == "secondary_link" || routeName == "tertiary_link" || routeName == "residential") {
		return true;
	}
	else if (routeName == "track" || routeName == "construction" || routeName == "service") {
		return false;
	}
	else if (routeName == "cycleway") {
		return false;
	}
	else if (routeName == "trunk") {
		return true;
	}
	else if (routeName == "trunk_link") {
		return true;
	}
	else if (routeName == "living_street") {
		return true;
	}
	return false;
}

static bool isMainRoad(std::string routeName) {
	if (routeName == "footway" || routeName == "path" || routeName == "pedestrian"
		|| routeName == "steps" || routeName == "elevator" || routeName == "platform"
		|| routeName == "bridleway") {
		return false;
	}
	else if (routeName == "motorway" || routeName == "primary") {
		return true;
	}
	else if (routeName == "motorway_link" || routeName == "primary_link") {
		return false;
	}
	else if (routeName == "unclassified" || routeName == "tertiary" || routeName == "secondary"
		|| routeName == "secondary_link" || routeName == "tertiary_link" || routeName == "residential") {
		return false;
	}
	else if (routeName == "track" || routeName == "construction" || routeName == "service") {
		return false;
	}
	else if (routeName == "cycleway") {
		return false;
	}
	else if (routeName == "trunk") {
		return true;
	}
	else if (routeName == "trunk_link") {
		return false;
	}
	else if (routeName == "living_street") {
		return false;
	}
	return false;
}

void GuiRepresentation::Area::insertNode(const id_t id) {
	_nodes.push_back(id);
}

void GuiRepresentation::Area::insertWay(const id_t id) {
    const auto& way = MapData::instance().ways()[id];
    if (!way.hasTag("highway"))
        return;

    QPair<QColor, int> penParams = routePen(way.tagValue("highway"));
    for (size_t i = 0; i < way.refs().size() - 1; i++){
        const id_t nodeId1 = way.refs()[i];
        const id_t nodeId2 = way.refs()[i + 1];
        const Node& node1 = MapData::instance().nodes()[nodeId1];
        const Node& node2 = MapData::instance().nodes()[nodeId2];
        const auto& coords1 = node1.localCoords(MapData::instance().bounds());
        const auto& coords2 = node2.localCoords(MapData::instance().bounds());
        const double aspectRatio = MapData::instance().bounds().aspectRatio();
        _ways[penParams].append(QLineF{ImageSize * aspectRatio * coords1[1], ImageSize * coords1[0],
                                       ImageSize * aspectRatio * coords2[1], ImageSize * coords2[0]});
    }
}

const std::vector<id_t>& GuiRepresentation::Area::nodes() const {
	return _nodes;
}

const QHash<QPair<QColor, int>, QVector<QLineF>>& GuiRepresentation::Area::ways() const {
	return _ways;
}

GuiRepresentation::GuiRepresentation()
{
	// init grid
	for (size_t scale = 0; scale < SCALES_NUM; scale++) {
		_map.push_back({});
		const size_t areasNum2 = 1 << (scale << 1);
		for (size_t i = 0; i < areasNum2; i++)
			_map.back().push_back(Area{});
	}

	// assign nodes to the grid positions
	for (const Node& node : MapData::instance().nodes()) {
		const auto scalesAndCoords = MapData::instance().bounds().scalesAndCoords(node);
		for (size_t scale = 0; scale < scalesAndCoords.size(); scale++) {
			const auto& areaIndX = scalesAndCoords[scale][0];
			const auto& areaIndY = scalesAndCoords[scale][1];
			const size_t areasNum = 1 << scale;
			for (size_t i = areaIndX[0]; i <= areaIndX[1]; i++)
				for (size_t j = areaIndY[0]; j <= areaIndY[1]; j++)
					_map[scale][i * areasNum + j].insertNode(node.id());
		}
	}

	// assign ways to the grid positions: secondary roads et al. only appear when we zoom in.
	for (const Way& way : MapData::instance().ways()) {
		const auto scalesAndCoords = MapData::instance().bounds().scalesAndCoords(way);
		for (size_t scale = 0; scale < scalesAndCoords.size(); scale++) {
            if (!way.hasTag("highway") || scale < 2 && !isMainRoad(way.tagValue("highway")))
				continue;

            //if (skipType(way.tagValue("highway")))
            //    continue;

            if (scale < 4 && !isRoad(way.tagValue("highway")))
                continue;

			const auto& areaIndX = scalesAndCoords[scale][0];
			const auto& areaIndY = scalesAndCoords[scale][1];
			const size_t areasNum = 1 << scale;
			for (size_t i = areaIndX[0]; i <= areaIndX[1]; i++)
				for (size_t j = areaIndY[0]; j <= areaIndY[1]; j++)
					_map[scale][i * areasNum + j].insertWay(way.id());
		}
	}
}

const QHash<QPair<QColor, int>, QVector<QLineF>>& GuiRepresentation::waysToVisualize(const Bounds& bounds) const {
	const auto [scale, x, y] = MapData::instance().bounds().scaleAndCoords(bounds);
	return _map[scale][(1 << scale) * x + y].ways();
}

// simple linear search
id_t GuiRepresentation::closestPoint(double lat, double lon, const Bounds& bounds) const {
	id_t res = 0;
	auto temp = Node(0, lat, lon, {});

	double minDist = std::numeric_limits<double>().max();
	const auto [scale, x, y] = MapData::instance().bounds().scaleAndCoords(bounds);
	for (const id_t i : _map[scale][(1 << scale) * x + y].nodes())
		if (Node::distance(temp, MapData::instance().nodes()[i]) < minDist) {
			minDist = Node::distance(temp, MapData::instance().nodes()[i]);
			res = i;
		}
	return res;
}
