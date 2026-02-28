#include "mapData.h"
#include "myXmlParser.h"

#include <iostream>
#include <fstream>
#include <sstream>

struct TrafficSignalAssignment {
	id_t id;
	int cluster;
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
		
		const auto splitLine = split(line, '\t');
		p.id = std::stoi(splitLine[0]);
		p.cluster = std::stoi(splitLine[3]);

		points.push_back(p);
	}

	file.close();

	return points;
}

MapData::MapData() {
    auto res = parseXML("path\\to\\file");

	if (!res.has_value())
		return;

	auto& [nodes, ways, relations, bounds] = *res;

	_nodes = nodes;
	_ways = ways;
	_relations = relations;
	_bounds = bounds;

    const auto assignments = readTrafficSignalAssignments("path\\to\\file");
	for (const auto& assignment : assignments)
		_nodes[assignment.id].addTag("cluster", std::to_string(assignment.cluster));
}
