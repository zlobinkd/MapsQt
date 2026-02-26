#include "myXmlWriter.h"
#include "node.h"
#include "way.h"
#include "relation.h"
#include "bounds.h"

#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>

void xmlWriter::writeNode(std::ofstream& file, const Node& node) const {
	const std::string firstNodeString = "<node id=\"" + std::to_string(node.id()) +
										"\" lat=\"" + std::to_string(node._lat) +
										"\" lon=\"" + std::to_string(node._lon) + "\">";
	file << firstNodeString << std::endl;
	for (const auto& [key, value] : node._tags) {
		const std::string tagStr = "    <tag k=\"" + key + "\" v=\"" + value + "\"/>";
		file << tagStr << std::endl;
	}
	const std::string lastNodeStr = "</node>";
	file << lastNodeStr << std::endl;
}

void xmlWriter::writeWay(std::ofstream& file, const Way& way) const {
	const std::string firstWayString = "<way id=\"" + std::to_string(way.id()) + "\">";
	file << firstWayString << std::endl;

	for (const auto id : way.refs()) {
		const std::string refStr = "    <nd ref=\"" + std::to_string(id) + "\"/>";
		file << refStr << std::endl;
	}

	for (const auto& [key, value] : way._tags) {
		const std::string tagStr = "    <tag k=\"" + key + "\" v=\"" + value + "\"/>";
		file << tagStr << std::endl;
	}
	const std::string lastWayStr = "</way>";
	file << lastWayStr << std::endl;
}

void xmlWriter::writeRelation(std::ofstream& file, const Relation& relation) const {
	const std::string firstRelString = "<relation id=\"" + std::to_string(relation._id) + "\">";
	file << firstRelString << std::endl;
	
	const std::string nodeStr = "    <member type=\"node\" ref=\"" + std::to_string(relation._nodeViaId) + "\" role=\"via\"/>";
	const std::string fromStr = "    <member type=\"way\" ref=\"" + std::to_string(relation._wayFromId) + "\" role=\"from\"/>";
	const std::string toStr = "    <member type=\"way\" ref=\"" + std::to_string(relation._wayToId) + "\" role=\"to\"/>";

	file << nodeStr << std::endl;
	file << fromStr << std::endl;
	file << toStr << std::endl;

	file << "    <tag k=\"type\" v=\"restriction\"/>\n";

	const std::string restr = std::string("    <tag k=\"restriction\" v=\"") + 
								(relation.type() == Relation::RestrictionType::FORBIDDEN ? 
									"no_turn\"/>" : "only_turn\"/>");
	file << restr << std::endl;

	const std::string lastRelStr = "</relation>";
	file << lastRelStr << std::endl;
}

void xmlWriter::write(const std::string& filename,
					  const Nodes& nodes,
					  const Ways& ways,
					  const std::vector<Relation>& relations,
					  const Bounds& bounds) const {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Could not open your dumbass xml!" << std::endl;
		return;
	}

	const std::string boundsInfo = "<bounds minlat=\"" + std::to_string(bounds._minLat) + 
										"\" maxlat=\"" + std::to_string(bounds._maxLat) + 
										"\" minlon=\"" + std::to_string(bounds._minLon) + 
										"\" maxlon=\"" + std::to_string(bounds._maxLon) + "\"/>";

	file << boundsInfo << std::endl;

	for (const Node& node : nodes)
		writeNode(file, node);

	for (const Way& way : ways)
		writeWay(file, way);

	for (const Relation& rel : relations)
		writeRelation(file, rel);

	file.close();
}