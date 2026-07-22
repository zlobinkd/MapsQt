#pragma once

#include "core.h"

#include <vector>
#include <string>
#include <array>

class Bounds;

class xmlWriter;

// class for a map node. (point on a map)
class Node {
public:
	using Tags = std::vector<std::pair<std::string, std::string>>;

	Node() = delete;
	explicit Node(id_t id, double lat, double lon, const Tags& tags);

	id_t id() const;
	void setId(id_t id);
	// pixel coordinates in an image
	std::array<double, 2> localCoords(const Bounds& bounds) const;
	// distance between two nodes in meters
	static double distance(const Node& one, const Node& other);
    // point between nodes
    static Node pointOnLine(const Node& left, const Node& right, double progression);

	bool hasTag(const std::string& tag) const;
	bool hasTagValue(const std::string& tag) const;
    std::string tagValue(const std::string& tag) const;
	void addTag(const std::string& key, const std::string& val);

private:
	id_t _id;
	// latitude
	double _lat;
	// longtitude
	double _lon;
	// tags: describe what's there irl
	Tags _tags;

	friend class xmlWriter;
};

using Nodes = std::vector<Node>;
