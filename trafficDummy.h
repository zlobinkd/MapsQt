#pragma once

#include "core.h"

class TrafficObject;

// Dummy traffic object, which doesn't collect motion history and has no route
class TrafficDummy {
public:
	TrafficDummy(const TrafficDummy&) = default;
	TrafficDummy(const TrafficObject&);

	double speed() const { return _speed; }
	double progress() const { return _progress; }
private:
	double _speed;

	// value between 0. and 1.
	double _progress;
};