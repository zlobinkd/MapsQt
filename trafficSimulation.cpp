#include "trafficSimulation.h"

#include "trafficCar.h"
#include "trafficSignal.h"
#include "mapData.h"

#include <algorithm>

ConnectionLoad::ConnectionLoad(const Connection& connection) : _segment(connection) {}

void ConnectionLoad::append(const TrafficDummy& dummy) {
	_traffic.emplace_back(dummy);
}

void ConnectionLoad::sortTraffic() {
	std::sort(_traffic.begin(), _traffic.end(),
		[](const TrafficDummy& d1, const TrafficDummy& d2) { return d1.progress() < d2.progress(); });
}

std::optional<TrafficDummy> ConnectionLoad::findNext(const double progress) const {
	const auto& it = std::find_if(_traffic.begin(), _traffic.end(),
		[&progress](const TrafficDummy& d) {return d.progress() > progress; });
	if (it != _traffic.end())
		return *it;
	return std::nullopt;
}

void ConnectionLoad::reset() {
	_traffic.clear();
}

void TrafficSimulation::run() {
	for (size_t i = 0; i < 10000000; i++)
		updateStep();

	dump();
}

void TrafficSimulation::dump() const
{}

void TrafficSimulation::updateStep() {
	fillDummies();
	updateObjects();
	clearDummies();
	deleteOffMapObjects();
	addNewObjects();
}

std::optional<std::pair<TrafficDummy, double>> TrafficSimulation::findNextObject(const TrafficObject& object) const
{
	const auto route = object.remainingRoute();
	if (route.empty())
		return std::nullopt;

	double distance = 0.;
	for (size_t i = 0; i < route.size(); i++)
	{
		for (auto& connectionLoad : _dummies[route[i].from()]) {
			const bool isSameWay = connectionLoad.segment().to() == route[i].to()
				&& connectionLoad.segment().wayId() == object.currentSegment().wayId();

			if (!isSameWay)
				continue;

			if (i == 0) {
				const auto nextObstacle = connectionLoad.findNext(object.progressOnCurrentSegment());
				if (nextObstacle.has_value())
				{
					const double progressDiff = nextObstacle->progress() - object.progressOnCurrentSegment();
					distance = progressDiff * object.currentSegment().distance();
					return std::make_pair(*nextObstacle, distance);
				}

				distance += (1. - object.progressOnCurrentSegment()) * object.currentSegment().distance();
			}
			else {
				if (!connectionLoad.load().empty())
				{
					distance += connectionLoad.load().front().progress() * connectionLoad.segment().distance();
					return std::make_pair(connectionLoad.load().front(), distance);
				}

				distance += connectionLoad.segment().distance();
			}
		}
	}
	return std::nullopt;
}

void TrafficSimulation::fillDummies() {
	for (const auto& object : _objects)
	{
		if (!object.isObstacle() || !object.isOnMap())
			continue;

		for (auto& connectionLoad : _dummies[object.currentSegment().from()])
		{
			const auto& loadSegment = connectionLoad.segment();
			const auto& objectSegment = object.currentSegment();
			if (loadSegment.wayId() == objectSegment.wayId() && loadSegment.to() == objectSegment.to())
				connectionLoad.append(object);
		}
	}

	for (auto& nodeOutputLoads : _dummies)
		for (auto& connectionLoad : nodeOutputLoads)
			connectionLoad.sortTraffic();
}

void TrafficSimulation::updateObjects() {
	for (auto& object : _objects)
	{
		const auto nextObjInfo = findNextObject(object);
		if (!nextObjInfo.has_value())
			object.update(1e7, 100.);

		object.update(nextObjInfo->second, nextObjInfo->first.speed());
	}
}

void TrafficSimulation::clearDummies() {
	for (auto& nodeOutputLoads : _dummies)
		for (auto& load : nodeOutputLoads)
			load.reset();
}
void TrafficSimulation::addNewObjects() {}

void TrafficSimulation::deleteOffMapObjects() {
	std::vector<size_t> idsToDelete;

	for (size_t i = 0; i < _objects.size(); i++)
		if (!_objects[i].isOnMap())
			idsToDelete.push_back(i);

	std::reverse(idsToDelete.begin(), idsToDelete.end());

	for (size_t idToDelete : idsToDelete)
		_objects.erase(_objects.begin() + idToDelete, _objects.begin() + idToDelete + 1);
}