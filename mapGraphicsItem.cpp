#include <mapGraphicsItem.h>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsView>

MapGraphicsItem::MapGraphicsItem() {
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptHoverEvents(true);

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

void MapGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    // Your existing paint code
    updateBounds();
    const auto& waysToVisualize = _map.waysToVisualize(_bounds);

    painter->setBackground(Qt::lightGray);
    for (const auto& [pen, ways] : waysToVisualize.asKeyValueRange()) {
        painter->setPen(QPen(pen.first, pen.second));
        painter->drawLines(ways);
    }

    if (_startNodeId.has_value())
    {
        const auto pt = nodePosition(*_startNodeId);
        painter->setPen(QPen{Qt::green, 7});
        painter->drawPoint(pt);
    }

    if (_endNodeId.has_value())
    {
        const auto pt = nodePosition(*_endNodeId);
        painter->setPen(QPen{Qt::red, 7});
        painter->drawPoint(pt);
    }

    painter->setPen(QPen{Qt::red, 2});
    painter->drawLines(_route);
}

QRectF MapGraphicsItem::boundingRect() const {
    return QRectF{ 0, 0, MapData::instance().imageSizeX(), MapData::instance().imageSizeY() };
}

void MapGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        _isPanning = true;
        _lastPanPoint = event->pos();
        _lastScenePos = event->scenePos();  // Store scene position
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        event->accept();
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void MapGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (_isPanning) {
        // Calculate movement in scene coordinates for more consistent panning
        QPointF sceneDelta = event->scenePos() - _lastScenePos;

        // Move the item in the opposite direction of mouse movement
        setPos(pos() + sceneDelta);

        _lastScenePos = event->scenePos();
        _lastPanPoint = event->pos();

        event->accept();
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void MapGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && _isPanning) {
        _isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        const auto [lat, lon] = toLatLon(event->pos());
        if (!_startNodeId.has_value())
            _startNodeId = _map.closestPoint(lat, lon, _bounds);
        else if (!_endNodeId.has_value())
            _endNodeId = _map.closestPoint(lat, lon, _bounds);
        else
        {
            _startNodeId.reset();
            _endNodeId.reset();
            _route.clear();
        }

        if (_startNodeId.has_value() && _endNodeId.has_value())
        {
            const auto route = _map.shortestPath(*_startNodeId, *_endNodeId);
            _route.clear();
            for (const auto& connection : route)
            {
                const auto ptFrom = nodePosition(connection.from());
                const auto ptTo = nodePosition(connection.to());
                _route.emplace_back(QLineF{ptFrom, ptTo});
            }
        }

        qDebug() << _startNodeId.value_or(0) << _endNodeId.value_or(0);

        event->accept();
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void MapGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent* event) {
    // Zoom factor
    const qreal factor = event->delta() > 0 ? 1.5 : 2. / 3.;

    // Limit zoom range
    const qreal newZoom = _zoomFactor * factor;

    // Get mouse position in scene coordinates
    QPointF mouseScenePos = event->scenePos();

    // Get the item's current position and scale
    QPointF oldPos = this->pos();
    qreal oldScale = _zoomFactor;

    // Calculate where the mouse is in item coordinates BEFORE scaling
    // This is the key fix - we need to use the current transform
    QPointF mouseItemPos = (mouseScenePos - oldPos) / oldScale;

    // Apply the new scale
    _zoomFactor = newZoom;
    setScale(_zoomFactor);

    // Calculate new position so that the same item point stays under the mouse
    // mouseItemPos in scene coordinates after scaling = newPos + mouseItemPos * newZoom
    // We want this to equal mouseScenePos
    // So: newPos = mouseScenePos - mouseItemPos * newZoom
    QPointF newPos = mouseScenePos - (mouseItemPos * _zoomFactor);

    setPos(newPos);

    event->accept();
}

QRectF MapGraphicsItem::getVisibleItemRect() const {
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

void MapGraphicsItem::updateBounds() {
    const auto visibleRect = getVisibleItemRect();

    const auto [minLat, minLon] = toLatLon(visibleRect.bottomLeft());
    const auto [maxLat, maxLon] = toLatLon(visibleRect.topRight());

    _bounds = Bounds{minLat, maxLat, minLon, maxLon};
}

std::array<double, 2> MapGraphicsItem::toLatLon(const QPointF& pt) const {
    return MapData::instance().bounds().globalCoords(pt.y() / MapData::instance().imageSizeY(), pt.x() / MapData::instance().imageSizeX());
}

QPointF MapGraphicsItem::nodePosition(const id_t id) const {
    const auto& node = MapData::instance().nodes()[id];
    const auto [y, x] = node.localCoords(MapData::instance().bounds());
    return QPointF{MapData::instance().imageSizeX() * x, MapData::instance().imageSizeY() * y};
}
