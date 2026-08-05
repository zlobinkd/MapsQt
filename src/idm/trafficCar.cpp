#include "trafficCar.h"
#include "src/map/way.h"
#include "src/utils/settings.h"
#include "src/map/mapData.h"

#include <algorithm>
#include <cmath>
#include <numeric>

TrafficCar::TrafficCar(const std::vector<Connection>& route) : _route(route) {}

static inline double pow2(const double x) {
    return x * x;
}

static inline double pow4(const double x) {
    return pow2(x) * pow2(x);
}

void TrafficCar::update(const double distanceToNextObject, const double nextObjectSpeed) {
    const double minDesiredGap = Settings::instance().minDesiredGap();
    const double safeReactionTime = Settings::instance().safeReactionTime();
    const double maxAcceleration = Settings::instance().maxAcceleration();
    const double maxDeceleration = Settings::instance().maxDeceleration();

	const double speedDifference = _speed - nextObjectSpeed;
	const double desiredGap = minDesiredGap 
		+ std::max(0., 
			_speed * safeReactionTime 
			+ _speed * speedDifference / (2 * std::sqrt(maxAcceleration * maxDeceleration))
		);

    const double speedUpdate = maxAcceleration *
        (1
            - pow4(_speed / currentSegment().way().speedLimit())
            - pow2(desiredGap / distanceToNextObject)
		);

    const double speedUpdateClamped = std::max(-_speed, speedUpdate);

    const double dt = Settings::instance().sampleTime();
    const double positionAdvance = _speed * dt + speedUpdateClamped * dt / 2;
    _speed += speedUpdateClamped;
	if (positionAdvance < 0.)
		return;

    double distanceToSegmentEnd = (1. - _progressOnCurrentSegment) * currentSegment().distance();
    size_t index = _currentConnectionId;
    while (index < _route.size() && distanceToSegmentEnd <= positionAdvance)
    {
        index++;
        distanceToSegmentEnd += _route[index].distance();
    }

    if (index < _route.size()) {
        const double localPosition = positionAdvance - distanceToSegmentEnd + _route[index].distance();

        _currentConnectionId = index;
        _progressOnCurrentSegment = localPosition / currentSegment().distance();
	}
	else {
		_currentConnectionId = _route.size();
	}
}

double TrafficCar::speed() const {
	return _speed;
}

double TrafficCar::progressOnCurrentSegment() const {
	return _progressOnCurrentSegment;
}

Connection TrafficCar::currentSegment() const {
	return _currentConnectionId < _route.size() ? _route[_currentConnectionId] : _route.back();
}

Span<const Connection> TrafficCar::remainingRoute() const {
    return Span<const Connection>(&_route.front() + _currentConnectionId, _route.size() - _currentConnectionId);
}

bool TrafficCar::isOnMap() const {
	return _currentConnectionId < _route.size();
}

QPair<QPair<QColor, int>, QPointF> TrafficCar::point() const {
    const id_t from = currentSegment().from();
    const id_t to = currentSegment().to();
    const auto& node1 = MapData::instance().nodes()[from];
    const auto& node2 = MapData::instance().nodes()[to];
    const auto node = Node::pointOnLine(node1, node2, _progressOnCurrentSegment);

    const auto [y, x] = node.localCoords(MapData::instance().bounds());
    const auto pen = QPair<QColor, int>{Qt::black, 5};
    const auto point = QPointF{ MapData::instance().imageSizeX() * x, MapData::instance().imageSizeY() * y };
    return { pen, point };
}

std::vector<ScaleAreaInformation> TrafficCar::scaleAreaInfo() const {
    const id_t from = currentSegment().from();
    const id_t to = currentSegment().to();
    const auto& node1 = MapData::instance().nodes()[from];
    const auto& node2 = MapData::instance().nodes()[to];
    const auto node = Node::pointOnLine(node1, node2, _progressOnCurrentSegment);
    return ScaleAreaInformation::areaInfos(node);
}
