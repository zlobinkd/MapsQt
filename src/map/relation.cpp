#include "relation.h"

Relation::Relation(id_t id, const std::array<id_t, 2>& wayIds, id_t nodeId, RestrictionType type)
	: _id(id), _wayFromId(wayIds[0]), _wayToId(wayIds[1]), _nodeViaId(nodeId), _type(type) 
{}

std::tuple<id_t, id_t, id_t> Relation::refs() const {
	return { _wayFromId, _nodeViaId, _wayToId };
}

Relation::RestrictionType Relation::type() const {
	return _type;
}