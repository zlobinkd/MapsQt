#pragma once

#include "core.h"

#include <array>
#include <tuple>

class xmlWriter;

// relation between two map ways.
// either restrictive, or imperative.
// either cannot move from a to b, or can only move from a to b.
class Relation {
public:
	enum class RestrictionType {
		FORBIDDEN,
		ONLY_ALLOWED
	};

	Relation() = delete;
	Relation(id_t id, const std::array<size_t, 2>& wayIds, id_t nodeId, RestrictionType type);

	// from, via, to
	std::tuple<id_t, id_t, id_t> refs() const;
	RestrictionType type() const;

private:
	id_t _id;
	id_t _wayFromId;
	id_t _wayToId;
	id_t _nodeViaId;
	RestrictionType _type;

	friend class xmlWriter;
};