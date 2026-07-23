#include "guiRepresentation.h"

#include "mapData.h"
#include "highwayClassification.h"

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

void GuiRepresentation::Area::insertNode(const id_t id) {
	_nodes.push_back(id);
}

void GuiRepresentation::Area::insertWay(const id_t id, const size_t sizeMultiplier) {
    const auto& way = MapData::instance().ways()[id];
    if (!way.hasTag("highway"))
        return;

    QPair<QColor, int> penParams = routePen(way.tagValue("highway"));
    penParams.second *= sizeMultiplier;
    for (size_t i = 0; i < way.refs().size() - 1; i++){
        const id_t nodeId1 = way.refs()[i];
        const id_t nodeId2 = way.refs()[i + 1];
        const Node& node1 = MapData::instance().nodes()[nodeId1];
        const Node& node2 = MapData::instance().nodes()[nodeId2];
        const auto& coords1 = node1.localCoords(MapData::instance().bounds());
        const auto& coords2 = node2.localCoords(MapData::instance().bounds());
        const auto segment = QLineF{MapData::instance().imageSizeX() * coords1[1], MapData::instance().imageSizeY() * coords1[0],
                                    MapData::instance().imageSizeX() * coords2[1], MapData::instance().imageSizeY() * coords2[0]};

        // --- only for debug, remove if necessary ---
        /*if (const auto label = MapData::instance().synchroLabel(nodeId2, nodeId1))
        {
            if (label == 1)
                _ways[QPair<QColor, int>(Qt::red, 1)].append(segment);
            else
                _ways[QPair<QColor, int>(Qt::green, 1)].append(segment);
        } else if (const auto label = MapData::instance().synchroLabel(nodeId1, nodeId2))
        {
            if (label == 1)
                _ways[QPair<QColor, int>(Qt::red, 1)].append(segment);
            else
                _ways[QPair<QColor, int>(Qt::green, 1)].append(segment);
        } else*/
        _ways[penParams].append(segment);
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
	for (const Node& node : MapData::instance().nodes())
        for (const auto& areaInfo : ScaleAreaInformation::areaInfos(node))
			_map[areaInfo.scale][areaInfo.areaIndex()].insertNode(node.id());

	// assign ways to the grid positions: secondary roads et al. only appear when we zoom in.
	for (const Way& way : MapData::instance().ways()) {
		if (!way.hasTag("highway"))
			continue;

        for (const auto& areaInfo : ScaleAreaInformation::areaInfos(way)) {
			if (areaInfo.scale < 2 && !isMainRoad(way.tagValue("highway")))
				continue;

			if (areaInfo.scale < 4 && !isRoad(way.tagValue("highway")))
				continue;

            const size_t scaleSizeMultiplier = 1 << (SCALES_NUM - areaInfo.scale - 1);
            _map[areaInfo.scale][areaInfo.areaIndex()].insertWay(way.id(), scaleSizeMultiplier);
		}
	}
}

const QHash<QPair<QColor, int>, QVector<QLineF>>& GuiRepresentation::waysToVisualize(const Bounds& bounds) const {
	const auto info = bounds.scaleAndCoords();
	return _map[info.scale][info.areaIndex()].ways();
}

// simple linear search
id_t GuiRepresentation::closestPoint(const double lat, const double lon, const Bounds& bounds) const {
	id_t res = 0;
	auto temp = Node(0, lat, lon, {});

	double minDist = std::numeric_limits<double>().max();
	const auto info = bounds.scaleAndCoords();
	for (const id_t i : _map[info.scale][info.areaIndex()].nodes())
		if (Node::distance(temp, MapData::instance().nodes()[i]) < minDist) {
			minDist = Node::distance(temp, MapData::instance().nodes()[i]);
			res = i;
		}
	return res;
}
