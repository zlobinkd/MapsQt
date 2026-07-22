#include "dynamicGuiRepresentation.h"
#include "mapData.h"
#include "guiRepresentation.h"

qreal DynamicGuiRepresentation::ImgSizeX = GuiRepresentation::ImageSize * MapData::instance().bounds().aspectRatio();
qreal DynamicGuiRepresentation::ImgSizeY = GuiRepresentation::ImageSize;

void DynamicGuiRepresentation::Area::insertNode(const Node& node) {
    const auto [y, x] = node.localCoords(MapData::instance().bounds());
    _nodes[QPair<QColor, int>{Qt::black, 5}].push_back(QPointF{ ImgSizeX * x, ImgSizeY * y });
}

const QHash<QPair<QColor, int>, QVector<QPointF>>& DynamicGuiRepresentation::Area::nodes() const {
    return _nodes;
}

DynamicGuiRepresentation::DynamicGuiRepresentation(const std::vector<Node>& nodes)
{
    // init grid
    for (size_t scale = 0; scale < SCALES_NUM; scale++) {
        _map.push_back({});
        const size_t areasNum2 = 1 << (scale << 1);
        for (size_t i = 0; i < areasNum2; i++)
            _map.back().push_back(Area{});
    }

    // assign nodes to the grid positions
    for (const Node& node : nodes)
        for (const auto& areaInfo : ScaleAreaInformation::areaInfos(node))
            _map[areaInfo.scale][areaInfo.areaIndex()].insertNode(node);
}

const QHash<QPair<QColor, int>, QVector<QPointF>>& DynamicGuiRepresentation::pointsToVisualize(const Bounds& bounds) const {
    const auto info = bounds.scaleAndCoords();
    return _map[info.scale][info.areaIndex()].nodes();
}
