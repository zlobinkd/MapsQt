#include "myXmlParser.h"
#include "node.h"
#include "way.h"
#include "relation.h"
#include "bounds.h"

#include <map>
#include <iostream>
#include <fstream>
#include <stdexcept>

static bool contains(const std::string& one, const std::string& another) {
	return one.find(another) != std::string::npos;
}

static std::optional<double> stringToDouble(const std::string& s) {
	try {
		const double res = stod(s);
		return res;
	}
	catch (const std::exception&) {
		return std::nullopt;
	}
}

static std::optional<id_t> stringToIdT(const std::string& s) {
	try {
		const id_t res = stoll(s);
		return res;
	}
	catch (const std::exception&) {
		return std::nullopt;
	}
}

static std::string parseAttribute(const std::string& xmlLine, const std::string& attr) {
	const id_t pos = xmlLine.find(attr);
	if (pos == std::string::npos)
		return "";

	return xmlLine.substr(pos + attr.size(), xmlLine.find('"', pos + attr.size()) - pos - attr.size());
}

static std::optional<Node> parseNode(std::ifstream& file, const std::string& firstNodeLine) {
	const auto id = stringToIdT(parseAttribute(firstNodeLine, "id=\""));
	const auto lat = stringToDouble(parseAttribute(firstNodeLine, "lat=\""));
	const auto lon = stringToDouble(parseAttribute(firstNodeLine, "lon=\""));

	if (!id.has_value() || !lat.has_value() || !lon.has_value())
		return std::nullopt;

	// Parse tags (simplified - assumes tags are on separate lines)
	std::string currLine = firstNodeLine;
	Node::Tags tags;

	if (contains(currLine, "/>"))
		return Node{ *id, *lat, *lon, tags };

	while (!contains(currLine, "</node>") && std::getline(file, currLine) && !contains(currLine, "</node>")) {
		if (!contains(currLine, "<tag"))
			continue;

		const auto k = parseAttribute(currLine, "k=\"");
		const auto v = parseAttribute(currLine, "v=\"");
		tags.emplace_back(std::pair<std::string, std::string>{ k, v });
	}

	return Node{ *id, *lat, *lon, tags };
}

static std::optional<Way> parseWay(
		std::ifstream& file, 
		const std::string& firstWayLine, 
		const std::map<id_t, id_t>& nodeIndices) {
	const auto id = stringToIdT(parseAttribute(firstWayLine, "id=\""));
	if (!id.has_value())
		return std::nullopt;

	if (contains(firstWayLine, "/>"))
		return std::nullopt;

	Way::Tags tags;
	std::vector<id_t> refs;

	// Parse tags (simplified - assumes tags are on separate lines)
	std::string currLine = firstWayLine;
	while (!contains(currLine, "</way>") && std::getline(file, currLine) && !contains(currLine, "</way>")) {
		if (contains(currLine, "<tag"))
		{
			const auto k = parseAttribute(currLine, "k=\"");
			const auto v = parseAttribute(currLine, "v=\"");
			tags.emplace_back(std::pair<std::string, std::string>{ k, v });
		}
		else if (contains(currLine, "<nd")) {
			const auto ref = stringToIdT(parseAttribute(currLine, "ref=\""));
			if (!ref.has_value())
				return std::nullopt;

			refs.emplace_back(nodeIndices.at(*ref));
		}
	}

	return Way{ *id, refs, tags };
}

static std::optional<Relation> parseRelation(
		std::ifstream& file, 
		const std::string& firstRelLine, 
		const std::map<id_t, id_t>& nodeIndices,
		const std::map<id_t, id_t>& wayIndices) {
	const auto id = stringToIdT(parseAttribute(firstRelLine, "id=\""));
	if (!id.has_value())
		return std::nullopt;

	if (contains(firstRelLine, "/>"))
		return std::nullopt;

	std::optional<id_t> wayToId;
	std::optional<id_t> wayFromId;
	std::optional<id_t> nodeViaId;
	std::optional<Relation::RestrictionType> type;
	// Parse tags (simplified - assumes tags are on separate lines)
	std::string curLine = firstRelLine;
	while (!contains(curLine, "</relation>") && std::getline(file, curLine) && !contains(curLine, "</relation>")) {
		if (contains(curLine, "<tag"))
		{
			const auto k = parseAttribute(curLine, "k=\"");
			if (k != "restriction")
				continue;

			const auto v = parseAttribute(curLine, "v=\"");
			if (v.find("no_") != std::string::npos)
				type = Relation::RestrictionType::FORBIDDEN;
			else if (v.find("only_") != std::string::npos)
				type = Relation::RestrictionType::ONLY_ALLOWED;
		}
		else if (contains(curLine, "<member")) {
			const auto memberType = parseAttribute(curLine, "type=\"");
			const auto ref = stringToIdT(parseAttribute(curLine, "ref=\""));
			if (!ref.has_value())
				return std::nullopt;

			if (memberType == "way") {
				if (parseAttribute(curLine, "role=\"") == "from")
					wayFromId = ref;
				else if (parseAttribute(curLine, "role=\"") == "to")
					wayToId = ref;
			}
			else if (memberType == "node" && parseAttribute(curLine, "role=\"") == "via") {
				nodeViaId = ref;
			}
		}
	}

	if (wayFromId.has_value() && wayToId.has_value() && nodeViaId.has_value() && type.has_value()
        && wayIndices.find(*wayFromId) != wayIndices.end()
        && wayIndices.find(*wayToId) != wayIndices.end()
        && nodeIndices.find(*nodeViaId) != nodeIndices.end())
	{
		return Relation{ *id, {wayIndices.at(*wayFromId), wayIndices.at(*wayToId)}, nodeIndices.at(*nodeViaId), *type };
    }

	return std::nullopt;
}

std::optional<XmlOutputTuple> parseXML(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Could not open your dumbass xml!" << std::endl;
		return std::nullopt;
	}

	std::optional<double> minLat;
	std::optional<double> maxLat;
	std::optional<double> minLon;
	std::optional<double> maxLon;
	Nodes nodes;
	Ways ways;
	std::vector<Relation> relations;
	std::map<id_t, id_t> nodeIndices;
	std::map<id_t, id_t> wayIndices;
	id_t currentNodeIndex = 0;
	id_t currentWayIndex = 0;

	std::string line;
	while (std::getline(file, line)) {
		// Trim whitespace (simplified)
		line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
		line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

		if (line.empty())
			continue;

		if (contains(line, "<bounds")) {
			minLat = stringToDouble(parseAttribute(line, "minlat=\""));
			if (!minLat.has_value())
				return std::nullopt;

			maxLat = stringToDouble(parseAttribute(line, "maxlat=\""));
			if (!maxLat.has_value())
				return std::nullopt;

			minLon = stringToDouble(parseAttribute(line, "minlon=\""));
			if (!minLon.has_value())
				return std::nullopt;

			maxLon = stringToDouble(parseAttribute(line, "maxlon=\""));
			if (!maxLon.has_value())
				return std::nullopt;

		} else if (contains(line, "<node")) {
			const auto node = parseNode(file, line);
			if (!node.has_value())
				continue;

			nodeIndices[node->id()] = currentNodeIndex;
			auto reassignedNode = *node;
			reassignedNode.setId(currentNodeIndex);
			nodes.emplace_back(reassignedNode);
			currentNodeIndex++;
		} else if (contains(line, "<way")) {
			const auto way = parseWay(file, line, nodeIndices);
			if (!way.has_value())
				continue;

			wayIndices[way->id()] = currentWayIndex;
			auto reassignedWay = *way;
			reassignedWay.setId(currentWayIndex);
			ways.emplace_back(reassignedWay);
			currentWayIndex++;
		} else if (contains(line, "<relation")) {
			const auto relation = parseRelation(file, line, nodeIndices, wayIndices);
			if (!relation.has_value())
				continue;

			relations.emplace_back(*relation);
		}
	}

	file.close();

	if (!minLon.has_value() || !maxLon.has_value() || !minLat.has_value() || !maxLat.has_value())
		return std::nullopt;

	return std::tuple{ nodes, 
					   ways, 
					   relations, 
					   Bounds{*minLat, *maxLat, *minLon, *maxLon} 
					 };
}
