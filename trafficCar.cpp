#include "trafficCar.h"
#include "way.h"

#include <algorithm>
#include <cmath>
#include <numeric>

TrafficCar::TrafficCar(const std::vector<Connection>& route) : _route(route) {}

void TrafficCar::update(const double distanceToNextObject, const double nextObjectSpeed) {
	constexpr double minDesiredGap = 7.;
	constexpr double safeReactionTime = 1.5;
	constexpr double maxAcceleration = 1.4;
	constexpr double maxDeceleration = 2.;

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

	constexpr double dt = 0.1;
	const double positionAdvance = _speed * dt + speedUpdate * dt / 2;
	_speed += speedUpdate;
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