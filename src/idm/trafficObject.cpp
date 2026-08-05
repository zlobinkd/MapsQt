#include "trafficObject.h"

Span<const Connection> TrafficObject::remainingRoute() const {
    return Span<const Connection>{ nullptr, 0 };
}

bool TrafficObject::isOnMap() const {
	return true;
}

bool TrafficObject::isObstacle() const {
	return true;
}
