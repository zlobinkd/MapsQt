#pragma once

#include <QGraphicsItem>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QMutex>

#include <optional>

#include "map.h"
#include "mapData.h"

class MapGraphicsItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    MapGraphicsItem();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;

    Bounds bounds() const;

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

    // map impl
    Map _map;
    // Bounds for current viewport
    Bounds _bounds = MapData::instance().bounds();
    mutable QMutex _mutex;

    // item transformation
    qreal _zoomFactor = 1.0;
    QPointF _lastPanPoint;
    bool _isPanning = false;
    QPointF _lastScenePos;

    // route info
    std::optional<id_t> _startNodeId;
    std::optional<id_t> _endNodeId;
    QVector<QLineF> _route;
};
