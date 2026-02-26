#include "connection.h"
#include "way.h"
#include "node.h"
#include "mapData.h"

Connection::Connection(id_t wayId, id_t from, id_t to) 
	: _wayId(wayId), 
	_from(from), 
	_to(to), 
	_path({}), 
	_distance(Node::distance(MapData::instance().nodes()[from], MapData::instance().nodes()[to]))
{}

Connection::Connection(id_t wayId, id_t from, id_t to, const std::vector<id_t>& path)
	: _wayId(wayId), _from(from), _to(to), _path(path)
{
	const auto& nodes = MapData::instance().nodes();
	if (path.empty())
		_distance += Node::distance(nodes[from], nodes[to]);
	else {
		_distance += Node::distance(nodes[from], nodes[path.front()]);
		_distance += Node::distance(nodes[to], nodes[path.back()]);
		for (size_t i = 1; i < path.size(); i++)
			_distance += Node::distance(nodes[path[i - 1]], nodes[path[i]]);
	}
}

std::optional<Connection> Connection::create(const Connection& left, const Connection& right) {
	if (left.to() != right.from())
		return std::nullopt;

	return Connection(left, right);
}

Connection::Connection(const Connection& left, const Connection& right)
	: _wayId(left._wayId), _from(left.from()), _to(right.to()), _distance(left.distance() + right.distance()) 
{
	_path = left._path;
	_path.push_back(left.to()); // == right.from()
	_path.insert(_path.end(), right._path.begin(), right._path.end());
}

std::vector<Connection> Connection::explode() const {
	if (_path.empty())
		return { *this };

	std::vector<Connection> result;
	result.push_back(Connection{ _wayId, _from, _path.front(), {} });
	for (size_t i = 1; i < _path.size(); i++)
		result.push_back(Connection{ _wayId, _path[i - 1], _path[i], {} });

	result.push_back(Connection{ _wayId, _path.back(), _to, {} });
	return result;
}

const Way& Connection::way() const {
	return MapData::instance().ways()[_wayId];
}