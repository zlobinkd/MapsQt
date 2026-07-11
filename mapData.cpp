#include "mapData.h"
#include "myXmlParser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

struct TrafficSignalAssignment {
	id_t pointId;
	int cluster;
	std::map<id_t, int> neighborLabels;
};

static std::vector<std::string> split(const std::string& s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

static std::vector<TrafficSignalAssignment> readTrafficSignalAssignments(std::string filename) {
	std::vector<TrafficSignalAssignment> points;
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Error opening file!" << std::endl;
		return {};
	}

	std::string line;
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		TrafficSignalAssignment p;
		
		const auto splitLine = split(line, ',');
		p.pointId = std::stoi(splitLine[0]);
		p.cluster = std::stoi(splitLine[1]);

		for (size_t i = 1; i < splitLine.size() / 2; i++)
			p.neighborLabels[std::stoi(splitLine[2 * i])] = std::stoi(splitLine[2 * i + 1]);

		points.push_back(p);
	}

	file.close();

	return points;
}

MapData::MapData() {
    auto res = parseXML("C:\\MapsFiles\\mapMoscow3");

	if (!res.has_value())
		return;

	auto& [nodes, ways, relations, bounds] = *res;

	_nodes = nodes;
	_ways = ways;
	_relations = relations;
	_bounds = bounds;
	_synchroIndex = std::vector<std::optional<size_t>>(_nodes.size());
    const auto assignments = readTrafficSignalAssignments("C:\\MapsFiles\\streetSignalsMoscow_clustered.csv");
	id_t currentTrafficSignalId = 0;
	for (const auto& assignment : assignments) {
		const auto it = std::find_if(_trafficSignalSynchros.begin(), _trafficSignalSynchros.end(),
			[&assignment](const auto& synchro) {return synchro.clusterId() == assignment.cluster; });
		if (assignment.cluster == -1 || it == _trafficSignalSynchros.end())
		{
			_synchroIndex[assignment.pointId] = _trafficSignalSynchros.size();
			_trafficSignalSynchros.push_back(TrafficSignalSynchro{ currentTrafficSignalId, assignment.cluster });
			currentTrafficSignalId++;
			_trafficSignalSynchros.back().append(assignment.pointId, assignment.neighborLabels);
		}
		else {
			_synchroIndex[assignment.pointId] = size_t(std::distance(_trafficSignalSynchros.begin(), it));
			it->append(assignment.pointId, assignment.neighborLabels);
		}
	}
}

std::optional<int> MapData::synchroLabel(const id_t node, const id_t neighbor) const {
	const auto& synchroIndex = _synchroIndex[node];
	if (!synchroIndex.has_value())
		return std::nullopt;

	return _trafficSignalSynchros[*synchroIndex].synchroInfo(node, neighbor);
}
