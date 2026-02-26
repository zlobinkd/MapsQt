#pragma once

#include "core.h"

#include "trafficObject.h"

// class for simulating traffic signals.
class TrafficSignal : public TrafficObject {
public:
	TrafficSignal(Connection segment, size_t counterStartPhase, size_t redTime, size_t greenTime);
	~TrafficSignal() override = default;

	// update signal (red/ green)
	void update(double distanceToNextObject, double nextObjectSpeed) override;
	double speed() const override;
	// 1. per definition (at the end of its Connection)
	double progressOnCurrentSegment() const override;
	Connection currentSegment() const override;

	// is red signal?
	virtual bool isObstacle() const override;

private:
	const Connection _segment;
	// internal clock
	size_t _timeCounter;
	// red phase time period
	const size_t _redTime;
	// green phase time period
	const size_t _greenTime;
};