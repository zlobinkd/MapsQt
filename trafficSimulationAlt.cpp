#include "TrafficSimulationAlt.h"

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

ConnectionLoadAlt::ConnectionLoadAlt(const Connection& connection) : _segment(connection) {}

void ConnectionLoadAlt::append(std::shared_ptr<TrafficCar> car) {
    _traffic.emplace_back(car);
}

void ConnectionLoadAlt::setTrafficSignal(std::shared_ptr<TrafficSignal> signal) {
    _trafficSignal = signal;
}

void ConnectionLoadAlt::updateTrafficSignal() {
    if (_trafficSignal != nullptr)
        _trafficSignal->update(0., 0.);
}

std::shared_ptr<TrafficCar> ConnectionLoadAlt::pop_front() {
    if (_traffic.empty())
        return nullptr;

    const auto car = _traffic.front();
    _traffic.pop_front();
    return car;
}

TrafficSimulationAlt::TrafficSimulationAlt(DynamicMapGraphicsItem* item) : _pathFinder(isHighway), _graphicsItem(item) {
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        _randomNodeGenerator.push_back(Random{ crossroadsNodes(), i });
}

void TrafficSimulationAlt::run() {
    initObjectsContainer();
    addTrafficSignals();
    executeAndShowElapsedTime([&](){ addCarsParallel(); }, "Add cars parallel");
	for (size_t i = 0; i < 10000000; i++)
    {
		updateStep();
        dump();
    }
}

static std::optional<QPair<QPair<QColor, int>, QPointF>> pointOnScreen(const TrafficObject& object, const ScaleAreaInformation& info) {
    const auto scalesAndCoords = object.scaleAreaInfo();
    const bool isPtVisible = std::find_if(
                                 scalesAndCoords.begin(), scalesAndCoords.end(),
                                 [&info](const auto& el){ return el == info; })
                             != scalesAndCoords.end();

    if (!isPtVisible)
        return std::nullopt;

    return object.point();
}

void TrafficSimulationAlt::dump() const
{
    QHash<QPair<QColor, int>, QVector<QPointF>> pts;
    const auto scaleAndCoords = _graphicsItem->bounds().scaleAndCoords();
    for (const auto& connections : _objects)
    {
        for (const auto& connectionLoad : connections)
        {
            if (const auto trafficSignal = connectionLoad.trafficSignal())
                if (const auto pt = pointOnScreen(*trafficSignal, scaleAndCoords))
                    pts[pt->first].push_back(pt->second);

            for (const auto& car : connectionLoad.load())
                if (const auto pt = pointOnScreen(*car, scaleAndCoords))
                    pts[pt->first].push_back(pt->second);
        }
    }

    _graphicsItem->updateData(std::move(pts));
}

void TrafficSimulationAlt::updateStep() {
    executeAndShowElapsedTime([&](){ addCars(); }, "Add cars");
    executeAndShowElapsedTime([&](){ updateCars(); }, "Update cars");
    executeAndShowElapsedTime([&](){ updateTrafficSignals(); }, "Update traffic signals");
    executeAndShowElapsedTime([&](){ transferCars(); }, "Transfer cars");
}

std::optional<std::pair<const std::shared_ptr<const TrafficObject>, double>> TrafficSimulationAlt::findNextObject(std::deque<std::shared_ptr<TrafficCar>>::const_reverse_iterator objectIt,
                                                                                                      std::deque<std::shared_ptr<TrafficCar>>::const_reverse_iterator currentSegmentEndIt) const
{
    const auto& object = *objectIt;
    const auto route = object->remainingRoute();
    if (route.empty())
		return std::nullopt;

    double distance = 0.;
    for (size_t i = 0; i < route.size(); i++)
	{
        for (const auto& connectionLoad : _objects[route[i].from()]) {
            if (connectionLoad.segment() != route[i])
                continue;

            if (i == 0) {
                const auto nextObstacle = std::find_if(objectIt + 1, currentSegmentEndIt, [](const std::shared_ptr<TrafficCar>& car) { return car->isOnMap(); });
                if (nextObstacle != currentSegmentEndIt)
                {
                    const auto obstacle = *nextObstacle;
                    const double progressDiff = obstacle->progressOnCurrentSegment() - object->progressOnCurrentSegment();
                    distance = progressDiff * object->currentSegment().distance();
                    return std::make_pair(*nextObstacle, distance);
                }

                distance += (1. - object->progressOnCurrentSegment()) * object->currentSegment().distance();
                if (const auto trafficSignal = connectionLoad.trafficSignal())
                    if (trafficSignal->isObstacle())
                        return std::make_pair(trafficSignal, distance);
            }
            else {
                const auto nextObstacle = std::find_if(connectionLoad.load().rbegin(), connectionLoad.load().rend(), [](const std::shared_ptr<TrafficCar>& car) { return car->isOnMap(); });
                if (nextObstacle != connectionLoad.load().rend())
                {
                    const auto obstacle = *nextObstacle;
                    distance += obstacle->progressOnCurrentSegment() * connectionLoad.segment().distance();
                    return std::make_pair(connectionLoad.load().front(), distance);
                }

                distance += connectionLoad.segment().distance();
                if (const auto trafficSignal = connectionLoad.trafficSignal())
                    if (trafficSignal->isObstacle())
                        return std::make_pair(trafficSignal, distance);

                if (distance > 1000.)
                    return std::nullopt;
            }
		}
	}
	return std::nullopt;
}

void TrafficSimulationAlt::initObjectsContainer() {
    const auto& nodes = MapData::instance().nodes();
    _objects = std::vector<Connections>(nodes.size());

    for (const auto& way : MapData::instance().ways()) {
        for (size_t i = 1; i < way.refs().size(); i++) {
            const id_t node1 = way.refs()[i - 1];
            const id_t node2 = way.refs()[i];
            _objects[node1].emplace_back(ConnectionLoadAlt{ Connection{ way.id(), node1, node2 } });
        }

        //if both ways not allowed
        if (way.tagValue("oneway") == "yes")
            continue;

        for (size_t i = 1; i < way.refs().size(); i++) {
            const id_t node1 = way.refs()[i - 1];
            const id_t node2 = way.refs()[i];
            _objects[node2].emplace_back(ConnectionLoadAlt{ Connection{ way.id(), node2, node1 } });
        }
    }
}

void TrafficSimulationAlt::updateCars() {
    for (auto& connections : _objects)
    {
        for (auto& connectionLoad : connections) {
            const auto& load = connectionLoad.loadRef();
            for (auto it = load.rbegin(); it != load.rend(); it++)
            {
                auto object = *it;
                if (const auto nextObjectInfo = findNextObject(it, load.rend()))
                    object->update(nextObjectInfo->second, nextObjectInfo->first->speed());
                else
                    object->update(10e7, 100.);
            }
        }
    }
}

void TrafficSimulationAlt::updateTrafficSignals() {
    for (auto& connections : _objects)
        for (auto& connectionLoad : connections)
            connectionLoad.updateTrafficSignal();
}

void TrafficSimulationAlt::addTrafficSignals() {
    const size_t singlePhaseDuration = Settings::instance().trafficLightSinglePhaseDuration() / Settings::instance().sampleTime();

    for (const auto& way : MapData::instance().ways())
    {
        for (size_t i = 0; i < way.refs().size() - 1; i++) {
            const id_t nodeId1 = way.refs()[i];
            const id_t nodeId2 = way.refs()[i + 1];

            if (const auto label = MapData::instance().synchroLabel(nodeId2, nodeId1))
            {
                for (auto& connectionLoad : _objects[nodeId1])
                {
                    if (connectionLoad.segment() != Connection{way.id(), nodeId1, nodeId2})
                        continue;

                    if (label == 1)
                        connectionLoad.setTrafficSignal(std::make_shared<TrafficSignal>(connectionLoad.segment(), singlePhaseDuration, singlePhaseDuration, singlePhaseDuration));
                    else
                        connectionLoad.setTrafficSignal(std::make_shared<TrafficSignal>(connectionLoad.segment(), singlePhaseDuration, singlePhaseDuration, singlePhaseDuration));
                }
            }

            if (const auto label = MapData::instance().synchroLabel(nodeId1, nodeId2))
            {
                for (auto& connectionLoad : _objects[nodeId2])
                {
                    if (connectionLoad.segment() != Connection{way.id(), nodeId2, nodeId1})
                        continue;

                    if (label == 1)
                        connectionLoad.setTrafficSignal(std::make_shared<TrafficSignal>(connectionLoad.segment(), singlePhaseDuration, singlePhaseDuration, singlePhaseDuration));
                    else
                        connectionLoad.setTrafficSignal(std::make_shared<TrafficSignal>(connectionLoad.segment(), singlePhaseDuration, singlePhaseDuration, singlePhaseDuration));
                }
            }
        }
    }
}

void TrafficSimulationAlt::addCars() {
    if (_currentSimulationPoolSize + 30 >= Settings::instance().simulationPoolSize())
        return;

    while (_currentSimulationPoolSize < Settings::instance().simulationPoolSize())
    {
        const id_t from = _randomNodeGenerator[0].rand();
        const id_t to = _randomNodeGenerator[0].rand();
        const auto path = _pathFinder.shortestPathBetweenCrossroads(from, to);
        if (path.empty())
            continue;

        for (auto& connectionLoad : _objects[path.front().from()])
            if (connectionLoad.segment() == path.front())
            {
                connectionLoad.append(std::make_shared<TrafficCar>(path));
                _currentSimulationPoolSize++;
            }

        //std::cout << _currentSimulationPoolSize << std::endl;
    }
}

void TrafficSimulationAlt::addCarsParallel() {
    const size_t numCarsToAdd = _currentSimulationPoolSize < Settings::instance().simulationPoolSize() ?
                                    Settings::instance().simulationPoolSize() - _currentSimulationPoolSize :
                                    0;
    const size_t numHardwareThreads = std::min((size_t)8, (size_t)std::thread::hardware_concurrency());
    const size_t threadPoolSize = numHardwareThreads > 2 ? numHardwareThreads - 2 : 1;
    const size_t numCarsPerThread = numCarsToAdd / threadPoolSize;
    QMutex mutex;

    const auto workerFunc = [&](const size_t threadNum) {
        std::vector<std::shared_ptr<TrafficCar>> cars;
        cars.reserve(numCarsPerThread);
        for (size_t j = 0; j < numCarsPerThread; j++) {
            const id_t from = _randomNodeGenerator[threadNum].rand();
            const id_t to = _randomNodeGenerator[threadNum].rand();
            const auto path = _pathFinder.shortestPathBetweenCrossroads(from, to);
            if (path.empty())
                continue;

            cars.push_back(std::make_shared<TrafficCar>(path));
        }
        QMutexLocker locker(&mutex);
        for (auto& car : cars)
            for (auto& connectionLoad : _objects[car->currentSegment().from()])
                if (connectionLoad.segment() == car->currentSegment())
                {
                    connectionLoad.append(car);
                    _currentSimulationPoolSize++;
                }
    };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < threadPoolSize; i++)
        threads.push_back(std::thread(std::bind(workerFunc, i)));

    for (auto& t : threads)
        t.join();
}

void TrafficSimulationAlt::transferCars() {
    for (auto& connections : _objects)
        for (auto& connectionLoad : connections)
        {
            auto& load = connectionLoad.loadRef();
            while (!load.empty() && !load.front()->isOnMap())
                load.pop_front();

            while (!load.empty() && load.front()->currentSegment() != connectionLoad.segment())
            {
                const auto carToTransfer = connectionLoad.pop_front();
                for (auto& transferConnection : _objects[carToTransfer->currentSegment().from()])
                    if (transferConnection.segment() == carToTransfer->currentSegment())
                        transferConnection.append(carToTransfer);
            }
        }

}
