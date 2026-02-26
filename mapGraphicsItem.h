#pragma once

#include <QGraphicsItem>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <optional>

#include "map.h"
#include "mapData.h"

class MapGraphicsItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    MapGraphicsItem();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override{ return QRectF{0, 0, GuiRepresentation::ImageSize, GuiRepresentation::ImageSize}; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;

private:
    QRectF getVisibleItemRect() const;
    void updateBounds();

    std::array<double, 2> toLatLon(const QPointF& pt) const;
    QPointF nodePosition(id_t) const;

    // size of map in item coordinates
    static qreal ImgSizeX;
    static qreal ImgSizeY;

    // map impl
    Map _map;
    // Bounds for current viewport
    Bounds _bounds = MapData::instance().bounds();

    // item transformation
    qreal _zoomFactor = 1.0;
    QPointF _lastPanPoint;
    bool _isPanning = false;
    QPointF _lastScenePos;

    // route info
    std::optional<id_t> _startNodeId;
    std::optional<id_t> _endNodeId;
    QVector<QLineF> _route;

    QHash<QPair<QColor, int>, QVector<QPointF>> _trafficSignalLocations;
};
