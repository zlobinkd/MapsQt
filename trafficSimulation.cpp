#include "trafficSimulation.h"

#include "trafficCar.h"
#include "trafficSignal.h"
#include "mapData.h"
#include "highwayClassification.h"
#include "settings.h"
#include "benchmark.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <QMutex>

static bool isHighway(const Way& way) {
    if (!way.hasTag("highway"))
        return false;

    return isRoad(way.tagValue("highway"));
}

static std::vector<id_t> crossroadsNodes() {
    std::vector<id_t> res;
    std::vector<size_t> nodeConnectionNum(MapData::instance().nodes().size(), 0);
    for (const auto& way : MapData::instance().ways())
        if (isHighway(way))
        {
            const auto& nodes = way.refs();
            for (size_t i = 0; i < nodes.size(); i++)
                if (i == 0 || i + 1 == nodes.size())
                    nodeConnectionNum[nodes[i]] += 1;
                else
                    nodeConnectionNum[nodes[i]] += 2;
        }

    for (id_t i = 0; i < nodeConnectionNum.size(); i++)
        if (nodeConnectionNum[i] != 2 && nodeConnectionNum[i] != 0)
            res.push_back(i);

    return res;
}

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

TrafficSimulation::TrafficSimulation(DynamicMapGraphicsItem* item) : _pathFinder(isHighway), _graphicsItem(item) {
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        _randomNodeGenerator.push_back(Random{ crossroadsNodes(), i });
}

void TrafficSimulation::run() {
    addTrafficSignals();
    initDummies();
    executeAndShowElapsedTime([&](){ addCarsParallel(); }, "Add cars parallel");
	for (size_t i = 0; i < 10000000; i++)
    {
		updateStep();
        dump();
    }
}

void TrafficSimulation::dump() const
{
    QHash<QPair<QColor, int>, QVector<QPointF>> pts;
    const auto scaleAndCoords = _graphicsItem->bounds().scaleAndCoords();
    for (const auto& object : _objects)
    {
        const auto scalesAndCoords = object->scaleAreaInfo();
        const bool isPtVisible = std::find_if(scalesAndCoords.begin(), scalesAndCoords.end(),
                                              [&scaleAndCoords](const auto& el){ return el == scaleAndCoords; })
                                 != scalesAndCoords.end();

        if (!isPtVisible)
            continue;

        const auto pt = object->point();
        pts[pt.first].push_back(pt.second);
    }

    _graphicsItem->updateData(std::move(pts));
}

void TrafficSimulation::updateStep() {
    executeAndShowElapsedTime([&](){ addCars(); }, "Add cars");
    executeAndShowElapsedTime([&](){ fillDummies(); }, "Fill dummies");
    executeAndShowElapsedTime([&](){ updateObjects(); }, "Update objects");
    executeAndShowElapsedTime([&](){ clearDummies(); }, "Clear dummies");
    executeAndShowElapsedTime([&](){ deleteOffMapObjects(); }, "Delete off map objects");
}

std::optional<std::pair<TrafficDummy, double>> TrafficSimulation::findNextObject(const TrafficObject& object) const
{
    const auto route = object.remainingRoute();
    if (route.empty())
		return std::nullopt;

    double distance = 0.;
    for (size_t i = 0; i < route.size(); i++)
	{
        for (const auto& connectionLoad : _dummies[route[i].from()]) {
            if (connectionLoad.segment() != route[i])
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

                if (distance > 1000.)
                    return std::nullopt;
            }
		}
	}
	return std::nullopt;
}

void TrafficSimulation::initDummies() {
    const auto& nodes = MapData::instance().nodes();
    _dummies = std::vector<Connections>(nodes.size());

    for (const auto& way : MapData::instance().ways()) {
        for (size_t i = 1; i < way.refs().size(); i++) {
            const id_t node1 = way.refs()[i - 1];
            const id_t node2 = way.refs()[i];
            _dummies[node1].emplace_back(ConnectionLoad{ Connection{ way.id(), node1, node2 } });
        }

        //if both ways not allowed
        if (way.tagValue("oneway") == "yes")
            continue;

        for (size_t i = 1; i < way.refs().size(); i++) {
            const id_t node1 = way.refs()[i - 1];
            const id_t node2 = way.refs()[i];
            _dummies[node2].emplace_back(ConnectionLoad{ Connection{ way.id(), node2, node1 } });
        }
    }

    for (const auto& object : _objects)
    {
        if (!object->isObstacle() || !object->isOnMap())
            continue;

        for (auto& connectionLoad : _dummies[object->currentSegment().from()])
        {
            const auto& loadSegment = connectionLoad.segment();
            const auto& objectSegment = object->currentSegment();
            if (loadSegment == objectSegment)
                connectionLoad.append(*object);
        }
    }

    for (auto& nodeOutputLoads : _dummies)
        for (auto& connectionLoad : nodeOutputLoads)
            connectionLoad.sortTraffic();
}

void TrafficSimulation::fillDummies() {
    for (const auto& object : _objects)
    {
        if (!object->isObstacle() || !object->isOnMap())
            continue;

        for (auto& connectionLoad : _dummies[object->currentSegment().from()])
        {
            const auto& loadSegment = connectionLoad.segment();
            const auto& objectSegment = object->currentSegment();
            if (loadSegment == objectSegment)
                connectionLoad.append(*object);
        }
    }

    for (auto& nodeOutputLoads : _dummies)
        for (auto& connectionLoad : nodeOutputLoads)
            connectionLoad.sortTraffic();
}

void TrafficSimulation::updateObjects() {
    for (auto& object : _objects)
    {
        const auto nextObjInfo = findNextObject(*object);
        if (!nextObjInfo.has_value())
            object->update(1e7, 100.);
        else
            object->update(nextObjInfo->second, nextObjInfo->first.speed());
    }
}

void TrafficSimulation::clearDummies() {
    for (auto& nodeOutputLoads : _dummies)
        for (auto& load : nodeOutputLoads)
            load.reset();
}

void TrafficSimulation::addTrafficSignals() {
    const size_t singlePhaseDuration = Settings::instance().trafficLightSinglePhaseDuration() / Settings::instance().sampleTime();

    for (const auto& way : MapData::instance().ways())
    {
        for (size_t i = 0; i < way.refs().size() - 1; i++){
            const id_t nodeId1 = way.refs()[i];
            const id_t nodeId2 = way.refs()[i + 1];

            if (const auto label = MapData::instance().synchroLabel(nodeId2, nodeId1))
            {
                if (label == 1)
                    _objects.push_back(std::make_unique<TrafficSignal>(Connection{way.id(), nodeId1, nodeId2}, singlePhaseDuration, singlePhaseDuration, singlePhaseDuration));
                else
                    _objects.push_back(std::make_unique<TrafficSignal>(Connection{way.id(), nodeId1, nodeId2}, 0, singlePhaseDuration, singlePhaseDuration));
            }

            if (const auto label = MapData::instance().synchroLabel(nodeId1, nodeId2))
            {
                if (label == 1)
                    _objects.push_back(std::make_unique<TrafficSignal>(Connection{way.id(), nodeId2, nodeId1}, singlePhaseDuration, singlePhaseDuration, singlePhaseDuration));
                else
                    _objects.push_back(std::make_unique<TrafficSignal>(Connection{way.id(), nodeId2, nodeId1}, 0, singlePhaseDuration, singlePhaseDuration));
            }
        }
    }
}

void TrafficSimulation::addCars() {
    if (_objects.size() + 30 >= Settings::instance().simulationPoolSize())
        return;

    while (_objects.size() < Settings::instance().simulationPoolSize())
    {
        const id_t from = _randomNodeGenerator[0].rand();
        const id_t to = _randomNodeGenerator[0].rand();
        const auto path = _pathFinder.shortestPathBetweenCrossroads(from, to);
        if (path.empty())
            continue;

        _objects.push_back(std::make_unique<TrafficCar>(path));
        //std::cout << _objects.size() << std::endl;
    }
}

void TrafficSimulation::addCarsParallel() {
    const size_t numCarsToAdd = _objects.size() < Settings::instance().simulationPoolSize() ?
                                    Settings::instance().simulationPoolSize() - _objects.size() :
                                    0;
    const size_t numHardwareThreads = std::min((size_t)8, (size_t)std::thread::hardware_concurrency());
    const size_t threadPoolSize = numHardwareThreads > 2 ? numHardwareThreads - 2 : 1;
    const size_t numCarsPerThread = numCarsToAdd / threadPoolSize;
    QMutex mutex;

    const auto workerFunc = [&](const size_t threadNum) {
        std::vector<std::unique_ptr<TrafficObject>> cars;
        cars.reserve(numCarsPerThread);
        for (size_t j = 0; j < numCarsPerThread; j++) {
            const id_t from = _randomNodeGenerator[threadNum].rand();
            const id_t to = _randomNodeGenerator[threadNum].rand();
            const auto path = _pathFinder.shortestPathBetweenCrossroads(from, to);
            if (path.empty())
                continue;

            cars.push_back(std::make_unique<TrafficCar>(path));
        }
        QMutexLocker locker(&mutex);
        for (auto& car : cars)
            _objects.emplace_back(std::move(car));
    };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < threadPoolSize; i++)
        threads.push_back(std::thread(std::bind(workerFunc, i)));

    for (auto& t : threads)
        t.join();
}

void TrafficSimulation::deleteOffMapObjects() {
    std::vector<size_t> idsToDelete;

    for (size_t i = 0; i < _objects.size(); i++)
        if (!_objects[i]->isOnMap())
            idsToDelete.push_back(i);

    std::reverse(idsToDelete.begin(), idsToDelete.end());

    for (size_t idToDelete : idsToDelete)
        _objects.erase(_objects.begin() + idToDelete, _objects.begin() + idToDelete + 1);
}
