#include "trafficSignal.h"
#include "mapData.h"

TrafficSignal::TrafficSignal(Connection segment, size_t counterStartPhase, size_t redTime, size_t greenTime)
	: _segment(segment), _timeCounter(counterStartPhase), _redTime(redTime), _greenTime(greenTime)
{}

void TrafficSignal::update(double, double)
{
	_timeCounter++;
	if (_timeCounter >= _redTime + _greenTime)
		_timeCounter = 0;
}

double TrafficSignal::speed() const {
	return 0.;
}

double TrafficSignal::progressOnCurrentSegment() const {
	return 1.;
}
Connection TrafficSignal::currentSegment() const {
	return _segment;
}

bool TrafficSignal::isObstacle() const {
	return _timeCounter < _redTime;
}

QPair<QPair<QColor, int>, QPointF> TrafficSignal::point() const {
    const id_t from = currentSegment().from();
    const id_t to = currentSegment().to();
    const auto& node1 = MapData::instance().nodes()[from];
    const auto& node2 = MapData::instance().nodes()[to];

    const double segmentDistance = Node::distance(node1, node2);
    const double pos = 1. - 3. / segmentDistance;

    const auto node = Node::pointOnLine(node1, node2, pos);

    const auto [y, x] = node.localCoords(MapData::instance().bounds());
    const auto color = isObstacle() ? Qt::red : Qt::green;
    const auto pen = QPair<QColor, int>{color, 5};
    const auto point = QPointF{ MapData::instance().imageSizeX() * x, MapData::instance().imageSizeY() * y };
    return { pen, point };
}

std::vector<ScaleAreaInformation> TrafficSignal::scaleAreaInfo() const {
    const id_t to = currentSegment().to();
    const auto& node = MapData::instance().nodes()[to];
    return ScaleAreaInformation::areaInfos(node);
}
