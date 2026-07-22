#pragma once

#include "node.h"
#include "way.h"
#include "bounds.h"

#include "QVector"
#include "QPen"
#include "QPointF"

#include "hash.h"

// map representation, optimized for GUI.
class DynamicGuiRepresentation {
public:
    DynamicGuiRepresentation(const std::vector<Node>& nodes);

    // ways from the grid position according to current image boundaries
    const QHash<QPair<QColor, int>, QVector<QPointF>>& pointsToVisualize(const Bounds&) const;

    constexpr static int ImageSize = 25000;

private:
    // grid position entry.
    class Area {
    public:
        Area() = default;

        void insertNode(const Node&);
        const QHash<QPair<QColor, int>, QVector<QPointF>>& nodes() const;

    private:
        QHash<QPair<QColor, int>, QVector<QPointF>> _nodes;
    };

    // size of map in item coordinates
    static qreal ImgSizeX;
    static qreal ImgSizeY;

    // grid.
    using MapImpl = std::vector<std::vector<Area>>;

    MapImpl _map;
};
