#pragma once

#include "core.h"

#include "trafficObject.h"
#include "trafficCar.h"
#include "trafficSignal.h"
#include "connection.h"
#include "graphRepresentation.h"
#include "random.h"
#include "dynamicMapGraphicsItem.h"

#include <vector>
#include <memory>
#include <deque>

// class that represents which objects are located on a certain segment
class ConnectionLoadAlt {
public:
    ConnectionLoadAlt(const Connection&);

    void append(const std::shared_ptr<TrafficCar>);
    inline const std::deque<std::shared_ptr<TrafficCar>>& load() const { return _traffic; }
    inline std::deque<std::shared_ptr<TrafficCar>>& loadRef() { return _traffic; }
    inline const Connection& segment() const { return _segment; }
    void setTrafficSignal(std::shared_ptr<TrafficSignal> signal);
    void updateTrafficSignal();
    inline std::shared_ptr<const TrafficSignal> trafficSignal() const { return _trafficSignal; }
    std::shared_ptr<TrafficCar> pop_front();
private:
	Connection _segment;
	// objects that are on _segment
    std::deque<std::shared_ptr<TrafficCar>> _traffic;
    std::shared_ptr<TrafficSignal> _trafficSignal;
};

// class for traffic simulation
class TrafficSimulationAlt {
public:
    TrafficSimulationAlt(DynamicMapGraphicsItem* item);

	// run simulation
	void run();
	// write result to a file
	void dump() const;
	
private:
	// 1 timestamp of the simulation
	void updateStep();
	// find the closest obstacle on the route of the given object
    std::optional<std::pair<const std::shared_ptr<const TrafficObject>, double>> findNextObject(
        std::deque<std::shared_ptr<TrafficCar>>::const_reverse_iterator objectIt,
        std::deque<std::shared_ptr<TrafficCar>>::const_reverse_iterator currentSegmentEndIt) const;

    // init dummy container
    void initObjectsContainer();
	// update traffic objects
    void updateCars();
    void updateTrafficSignals();
    // transfer cars to their next segments
    void transferCars();

    // adds all traffic signals to the simulation
    void addTrafficSignals();
    // randomly spawns cars
    void addCars();
    void addCarsParallel();

	// collection of traffic that moves from the same node
    using Connections = std::vector<ConnectionLoadAlt>;

    // vector index == segment.from.id
    std::vector<Connections> _objects;

    GraphRepresentation _pathFinder;
    std::vector<Random> _randomNodeGenerator;

    DynamicMapGraphicsItem* _graphicsItem;

    size_t _currentSimulationPoolSize = 0;
};
