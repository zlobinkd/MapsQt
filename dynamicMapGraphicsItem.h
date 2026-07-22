#pragma once

#include <QGraphicsItem>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QMutex>

#include "guiRepresentation.h"
#include "dynamicGuiRepresentation.h"
#include "mapGraphicsItem.h"

class DynamicMapGraphicsItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    DynamicMapGraphicsItem(const MapGraphicsItem* const staticMapItem);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override { return QRectF{ 0, 0, GuiRepresentation::ImageSize, GuiRepresentation::ImageSize }; }

    void updateData(DynamicGuiRepresentation&& newData);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void dataUpdated();

private slots:
    void onDataUpdated();

private:
    QRectF getVisibleItemRect() const;

    // dynamic points on the map
    DynamicGuiRepresentation _data;
    mutable QMutex _mutex;

    const MapGraphicsItem* const _staticMapItem;
    std::atomic<bool> _updatePending{false};
};
