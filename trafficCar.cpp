#include "trafficCar.h"
#include "way.h"
#include "settings.h"
#include "mapData.h"

#include <algorithm>
#include <cmath>
#include <numeric>

TrafficCar::TrafficCar(const std::vector<Connection>& route) : _route(route) {}

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
            - std::pow(_speed / currentSegment().way().speedLimit(), 4)
            - std::pow(desiredGap / distanceToNextObject, 2)
		);

    const double speedUpdateClamped = std::max(-_speed, speedUpdate);

    const double dt = Settings::instance().sampleTime();
    const double positionAdvance = _speed * dt + speedUpdateClamped * dt / 2;
    _speed += speedUpdateClamped;
	if (positionAdvance < 0.)
		return;

	std::vector<double> nextSegmentDistances;
	for (size_t i = _currentConnectionId; i < _route.size(); i++) {
		if (i == _currentConnectionId)
			nextSegmentDistances.emplace_back((1. - _progressOnCurrentSegment) * currentSegment().distance());
		else
			nextSegmentDistances.emplace_back(_route[i].distance());
	}

	auto it = std::find_if(nextSegmentDistances.begin(), nextSegmentDistances.end(),
		[&positionAdvance](const double x) { return x > positionAdvance; });

	if (it != nextSegmentDistances.end()) {
		size_t index = std::distance(nextSegmentDistances.begin(), it);
		const double localPositionAdvance = positionAdvance 
			- std::accumulate(nextSegmentDistances.begin(), nextSegmentDistances.begin() + index, 0);

		_currentConnectionId += index;
		if (_currentConnectionId >= _route.size())
			return;

		if (index > 0)
			_progressOnCurrentSegment = localPositionAdvance / currentSegment().distance();
		else
			_progressOnCurrentSegment += localPositionAdvance / currentSegment().distance();
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

const std::vector<Connection> TrafficCar::remainingRoute() const {
	return std::vector<Connection>(_route.begin() + _currentConnectionId, _route.end());
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
