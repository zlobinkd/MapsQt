#include "trafficObject.h"

const std::vector<Connection> TrafficObject::remainingRoute() const {
	return {};
}

bool TrafficObject::isOnMap() const {
	return true;
}

bool TrafficObject::isObstacle() const {
	return true;
}