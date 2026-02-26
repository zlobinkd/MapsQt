#pragma once

#include <optional>
#include <vector>

#include "core.h"
#include "relation.h"

class Way;
class Node;

// A class defining a graph edge.
class Connection {
public:
	Connection() = delete;
	// create simple connection between two Nodes
	Connection(const id_t wayId, id_t from, id_t to);
	// merge connections
	Connection(const id_t wayId, id_t from, id_t to, const std::vector<id_t>& path);

	// merge connections
	// left.to() must be equal to right.from()
	static std::optional<Connection> create(const Connection& left, const Connection& right);

	// returns all simple connections along _path
	std::vector<Connection> explode() const;

	inline id_t to() const { return _to; }
	inline id_t from() const { return _from; }
	// referenced way
	inline id_t wayId() const { return _wayId; }
	const Way& way() const;
	// points between <from> and <to>
	inline const std::vector<id_t>& path() const { return _path; }
	// distance from <from> to <to>, along the <path>
	inline double distance() const { return _distance; }

private:
	Connection(const Connection& left, const Connection& right);

	// All referenced nodes belong to the same Way
	id_t _wayId;
	// Node Ids
	id_t _from;
	id_t _to;
	std::vector<id_t> _path;

	// cached for performance
	double _distance = 0.;
};