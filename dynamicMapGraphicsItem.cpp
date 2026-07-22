#include "dynamicMapGraphicsItem.h"
#include "mapData.h"
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsView>

DynamicMapGraphicsItem::DynamicMapGraphicsItem(const MapGraphicsItem* const staticMapItem)
    : _data({}), _staticMapItem(staticMapItem) {
    setZValue(100);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptHoverEvents(false);
    connect(this, &DynamicMapGraphicsItem::dataUpdated,
            this, &DynamicMapGraphicsItem::onDataUpdated,
            Qt::QueuedConnection);
}

void DynamicMapGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QMutexLocker locker(&_mutex);

    painter->save();
    painter->setTransform(_staticMapItem->sceneTransform(), true);

    for (const auto& [pen, points] : _data.asKeyValueRange()) {
        painter->setPen(QPen(pen.first, pen.second));
        painter->drawPoints(points);
    }

    painter->restore();
}

QRectF DynamicMapGraphicsItem::boundingRect() const {
    return QRectF{ 0, 0, MapData::instance().imageSizeX(), MapData::instance().imageSizeY() };
}

Bounds DynamicMapGraphicsItem::bounds() const {
    return _staticMapItem->bounds();
}

void DynamicMapGraphicsItem::updateData(QHash<QPair<QColor, int>, QVector<QPointF>>&& data) {
    QMutexLocker locker(&_mutex);
    _data = std::move(data);
    emit dataUpdated();
}

void DynamicMapGraphicsItem::onDataUpdated() {
    update();
}

void DynamicMapGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    event->ignore();
}

QRectF DynamicMapGraphicsItem::getVisibleItemRect() const {
    if (!scene() || scene()->views().isEmpty()) {
        return QRectF();
    }

    // Get the first view (assuming single view)
    QGraphicsView* view = scene()->views().first();

    // Get visible area in viewport coordinates and map to scene
    QRect viewportRect = view->viewport()->rect();
    QRectF sceneRect = view->mapToScene(viewportRect).boundingRect();
    return mapFromScene(sceneRect).boundingRect();
}
