#include "trafficSignalSynchro.h"

TrafficSignalSynchro::TrafficSignalSynchro(const id_t id, const int clusterId) 
	: _id(id), _clusterId(clusterId) {}

void TrafficSignalSynchro::append(const id_t trafficSignal, const std::map<id_t, int>& subLabels) {
	_synchroInfo[trafficSignal] = subLabels;
}

std::optional<int> TrafficSignalSynchro::synchroInfo(const id_t pt, const id_t neighbor) const {
	const auto it = _synchroInfo.find(pt);
	if (it == _synchroInfo.end())
		return std::nullopt;

	const auto innerIt = it->second.find(neighbor);
	if (innerIt == it->second.end())
		return std::nullopt;

	return innerIt->second;
}