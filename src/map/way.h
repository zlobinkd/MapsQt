#pragma once

#include "src/core.h"

#include <vector>
#include <array>
#include <string>

class xmlWriter;
class Bounds;

// class for the map way.
// a map way is a sequence of nodes: a street, a house, a fence, etc.
class Way {
public:
	using Tags = std::vector<std::pair<std::string, std::string>>;

	Way() = delete;
	Way(id_t id, const std::vector<id_t>& nodeIds, const Tags& tags);

	id_t id() const;
	void setId(id_t id);

	bool hasTag(const std::string& tag) const;
    std::string tagValue(const std::string& tag) const;
	// referenced nodes
	const std::vector<id_t>& refs() const;

	inline double speedLimit() const { return _speedLimit; }

	double getSpeedLimit() const;

private:
	id_t _id;
	// referenced nodes
	std::vector<id_t> _nodeIds;
	// tags: street name, road type, etc.
	Tags _tags;

	double _speedLimit = 1.;

	friend class xmlWriter;
};

using Ways = std::vector<Way>;
