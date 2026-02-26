#pragma once

#include "core.h"

#include "trafficObject.h"
#include "trafficDummy.h"
#include "connection.h"

#include <vector>

// class that represents which objects are located on a certain segment
class ConnectionLoad {
public:
	ConnectionLoad(const Connection&);

	void append(const TrafficDummy&);
	void sortTraffic();
	const std::vector<TrafficDummy>& load() const { return _traffic; }
	// find first traffic dummy with progress higher than the given value
	std::optional<TrafficDummy> findNext(double progress) const;
	const Connection& segment() const { return _segment; }
	void reset();
private:
	Connection _segment;
	// objects that are on _segment
	std::vector<TrafficDummy> _traffic;
};

// class for traffic simulation
class TrafficSimulation {
public:
	// run simulation
	void run();
	// write result to a file
	void dump() const;
	
private:
	// 1 timestamp of the simulation
	void updateStep();
	// find the closest obstacle on the route of the given object
	std::optional<std::pair<TrafficDummy, double>> findNextObject(const TrafficObject&) const;
	// fill dummy container
	void fillDummies();
	// update traffic objects
	void updateObjects();
	
	void clearDummies();
	// add new traffic objects, if their number is too small
	void addNewObjects();
	// delete traffic objects that finished their routes
	void deleteOffMapObjects();

	// collection of traffic that moves from the same node
	using Connections = std::vector<ConnectionLoad>;

	// vector index == segment.from.id
	std::vector<Connections> _dummies;
	std::vector<TrafficObject> _objects;
};