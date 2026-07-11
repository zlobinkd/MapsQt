#pragma once

#include "core.h"

#include <map>
#include <optional>

class TrafficSignalSynchro {
public:
	TrafficSignalSynchro(id_t id, int clusterId);

	void append(id_t trafficSignal, const std::map<id_t, int>& subLabels);
	id_t id() const { return _id; }
	int clusterId() const { return _clusterId; }
	std::optional<int> synchroInfo(id_t pt, id_t neighbor) const;
private:
	id_t _id;
	int _clusterId;
	// node id -> neighbor ids with phase label
	std::map<id_t, std::map<id_t, int>> _synchroInfo;
};