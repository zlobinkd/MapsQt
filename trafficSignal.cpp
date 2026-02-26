#include "trafficSignal.h"

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