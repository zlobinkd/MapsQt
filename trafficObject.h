#pragma once

#include "core.h"
#include "connection.h"
#include "bounds.h"

#include <QPen>

// base class for objects that are simulated as traffic.
class TrafficObject {
public:
	virtual ~TrafficObject() = default;

	// update speed, position, state, ...
	virtual void update(double distanceToNextObject, double nextObjectSpeed) = 0;
	virtual double speed() const = 0;

	// relative position on the current Connection between <from> and <to>, value from 0. to 1.
	virtual double progressOnCurrentSegment() const = 0;
	virtual Connection currentSegment() const = 0;
	virtual const std::vector<Connection> remainingRoute() const;

	// is being simulated?
	virtual bool isOnMap() const;

	// is an obstacle for other traffic objects?
	virtual bool isObstacle() const;

    // visualization on the map
    virtual QPair<QPair<QColor, int>, QPointF> point() const = 0;

    virtual std::vector<ScaleAreaInformation> scaleAreaInfo() const = 0;
};
