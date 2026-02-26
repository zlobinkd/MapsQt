#include "graphRepresentation.h"

#include "mapData.h"

#include <algorithm>
#include <iostream>
#include <limits>

GraphRepresentation::GraphRepresentation()
{
	const auto& nodes = MapData::instance().nodes();
	_connections = std::vector<Connections>(nodes.size());
	_connectionRef = std::vector<std::set<id_t>>(nodes.size());

	for (const auto& way : MapData::instance().ways()) {
		for (size_t i = 1; i < way.refs().size(); i++) {
			const id_t node1 = way.refs()[i - 1];
			const id_t node2 = way.refs()[i];
			_connections[node1].output.emplace_back(Connection{ way.id(), node1, node2});
			_connections[node2].input.emplace_back(Connection{ way.id(), node1, node2});
		}

		//if both ways not allowed
		if (way.tagValue("oneway") == "yes")
			continue;

		for (size_t i = 1; i < way.refs().size(); i++) {
			const id_t node1 = way.refs()[i - 1];
			const id_t node2 = way.refs()[i];
			_connections[node2].output.emplace_back(Connection{ way.id(), node2, node1 });
			_connections[node1].input.emplace_back(Connection{ way.id(), node2, node1 });
		}
	}

	// under construction
	mergeNodes();
}

void GraphRepresentation::mergeNodes() {
	for (size_t i = 0; i < _connections.size(); i++)
		mergeNode(i);
}

std::set<id_t> GraphRepresentation::mergeNode(id_t i) {
	// not equal output and input: cannot remove node
	if (_connections[i].input.size() != _connections[i].output.size())
		return {};

	// no connections or more than two neighboring nodes: cannot remove
	if (_connections[i].input.size() == 0 || _connections[i].input.size() > 2)
		return {};

	// ways not the same: cannot remove (for safety)
	const id_t wayId = _connections[i].input.front().wayId();
	if (_connections[i].input.back().wayId() != wayId)
		return {};

	if (_connections[i].output.front().wayId() != wayId)
		return {};

	if (_connections[i].output.back().wayId() != wayId)
		return {};

	// align inputs and outputs
	// from  1->2, 2->1  make  1->2, 2->3
	//       3->2, 2->3        3->2, 2->1
	if (_connections[i].input.front().from() == _connections[i].output.front().to())
	{
		if (_connections[i].input.size() == 1)
			return {};
		std::reverse(_connections[i].output.begin(), _connections[i].output.end());
	}

	for (size_t j = 0; j < _connections[i].input.size(); j++) {
		id_t from = _connections[i].input[j].from();
		id_t to = _connections[i].output[j].to();

		// remove old connections.
		for (int k = _connections[from].output.size() - 1; k >= 0; k--)
			if (_connections[from].output[k].to() == i && _connections[from].output[k].wayId() == wayId)
				_connections[from].output.erase(_connections[from].output.begin() + k);

		for (int k = _connections[to].input.size() - 1; k >= 0; k--)
			if (_connections[to].input[k].from() == i && _connections[to].input[k].wayId() == wayId)
				_connections[to].input.erase(_connections[to].input.begin() + k);

		// add new connections.
		auto newConnection = Connection::create(_connections[i].input[j], _connections[i].output[j]);
		if (!newConnection.has_value())
		{
			std::cout << "Could not merge two connections!" << std::endl;
			return {};
		}
		_connections[from].output.push_back(*newConnection);
		_connections[to].input.push_back(*newConnection);

		// set new node references.
		for (id_t k : newConnection->path()) {
			_connectionRef[k].clear();
			_connectionRef[k].insert(from);
			_connectionRef[k].insert(to);
		}
	}

	// remove node... goodbye...
	_connections[i].input.clear();
	_connections[i].output.clear();

	return _connectionRef[i];
}

void GraphRepresentation::mergeNodes(id_t i) {
	std::set<id_t> ids = { i };
	while (!ids.empty()) {
		std::set<id_t> newIds;
		for (id_t j : ids)
			for (id_t k : mergeNode(j))
				newIds.insert(k);
		ids = newIds;
	}
}

void GraphRepresentation::unfoldNodes(id_t i) {
	// there must be the left and right referenced nodes
	if (_connectionRef[i].size() != 2)
		return;

	const id_t left = *_connectionRef[i].begin();
	const id_t right = *(_connectionRef[i].rbegin());

	for (const auto& connection : _connections[left].input) {
		if (connection.from() != right)
			continue;

		const auto& path = connection.path();
		if (std::find(path.begin(), path.end(), i) == path.end())
			continue;

		for (const auto& newConnection : connection.explode())
		{
			_connections[newConnection.to()].input.push_back(newConnection);
			_connections[newConnection.from()].output.push_back(newConnection);
			_connectionRef[newConnection.to()].clear();
		}
	}

	for (const auto& connection : _connections[left].output) {
		if (connection.to() != right)
			continue;

		const auto& path = connection.path();
		if (std::find(path.begin(), path.end(), i) == path.end())
			continue;

		for (const auto& newConnection : connection.explode())
		{
			_connections[newConnection.from()].output.push_back(newConnection);
			_connections[newConnection.to()].input.push_back(newConnection);
			_connectionRef[newConnection.from()].clear();
		}
	}

	if (_connectionRef[i].empty()) {
		for (int j = _connections[left].input.size() - 1; j >= 0; j--)
		{
			const auto& path = _connections[left].input[j].path();
			if (_connections[left].input[j].from() == right && std::find(path.begin(), path.end(), i) != path.end())
				_connections[left].input.erase(_connections[left].input.begin() + j);
		}

		for (int j = _connections[right].input.size() - 1; j >= 0; j--)
		{
			const auto& path = _connections[right].input[j].path();
			if (_connections[right].input[j].from() == left && std::find(path.begin(), path.end(), i) != path.end())
				_connections[right].input.erase(_connections[right].input.begin() + j);
		}

		for (int j = _connections[left].output.size() - 1; j >= 0; j--)
		{
			const auto& path = _connections[left].output[j].path();
			if (_connections[left].output[j].to() == right && std::find(path.begin(), path.end(), i) != path.end())
				_connections[left].output.erase(_connections[left].output.begin() + j);
		}

		for (int j = _connections[right].output.size() - 1; j >= 0; j--)
		{
			const auto& path = _connections[right].output[j].path();
			if (_connections[right].output[j].to() == left && std::find(path.begin(), path.end(), i) != path.end())
				_connections[right].output.erase(_connections[right].output.begin() + j);
		}
	}
}

std::vector<Connection> GraphRepresentation::shortestPath(id_t from, id_t to) {
	unfoldNodes(from);
	unfoldNodes(to);
	// init algorithm containers
	auto routeLengths = std::vector<double>(MapData::instance().nodes().size(), std::numeric_limits<double>().max());
	auto prevNodes = std::vector<std::pair<id_t, const Way*>>(MapData::instance().nodes().size(), { 0, nullptr });
	routeLengths[from] = 0.;
	prevNodes[from] = { from, nullptr };
	auto currentNodes = std::set<id_t>{ from };

	// Dijkstra
	while (!currentNodes.empty()) {
		auto nextNodes = std::set<id_t>{};
		for (const id_t node : currentNodes) {
			for (const auto& connection : _connections[node].output) {
				const double maxSpeed = connection.way().speedLimit();
				if (routeLengths[connection.to()] > routeLengths[connection.from()] + connection.distance() / maxSpeed) {
					routeLengths[connection.to()] = routeLengths[connection.from()] + connection.distance() / maxSpeed;
					prevNodes[connection.to()] = { connection.from(), &connection.way() };
					if (routeLengths[connection.to()] < routeLengths[to])
					{
						nextNodes.insert(connection.to());
					}
				}
			}
		}
		currentNodes = nextNodes;
	}

	// find the actual route
	std::vector<Connection> route;
	id_t currentNode = to;
	while (currentNode != from)
	{
		for (const auto& connection : _connections[currentNode].input) {
			if (connection.from() == prevNodes[currentNode].first && &connection.way() == prevNodes[currentNode].second) {
				const auto connectionsToAppend = connection.explode();
				route.insert(route.begin(), connectionsToAppend.begin(), connectionsToAppend.end());
				break;
			}
		}
		currentNode = prevNodes[currentNode].first;
	}

	mergeNodes(from);
	mergeNodes(to);

	return route;
}
