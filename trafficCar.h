#pragma once

#include "core.h"

#include "trafficObject.h"

// Class for car motion simulation
class TrafficCar : public TrafficObject {
public:
	TrafficCar(const std::vector<Connection>& route);
	~TrafficCar() override = default;

	// updates current position and speed
	void update(double distanceToNextObject, double nextObjectSpeed) override;
	double speed() const override;

	// on the current simple connection, shows how close the car is to the segment endpoint (value between 0. and 1.)
	double progressOnCurrentSegment() const override;
	// current simple connection (no internal path)
	Connection currentSegment() const override;
	const std::vector<Connection> remainingRoute() const override;
	// if the car reached its destination, it is no longer on the map
	bool isOnMap() const override;

private:
	const std::vector<Connection> _route;
	double _speed = 0.;

	// value between 0. and 1.
	double _progressOnCurrentSegment = 0.;
	// Connection index in _route
	size_t _currentConnectionId = 0;
};