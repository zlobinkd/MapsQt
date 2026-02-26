#pragma once

#include <set>

#include "node.h"
#include "way.h"
#include "connection.h"

// map representation, optimized for the route search.
class GraphRepresentation {
public:
	GraphRepresentation();

	// Dijkstra's algorithm
	std::vector<Connection> shortestPath(id_t from, id_t to);
private:
	struct Connections {
		std::vector<Connection> input;
		std::vector<Connection> output;
	};

	// better name? Eliminates the "chain" connections.
	// remove traversable graph vertices that only have two neighboring vertices.
	void mergeNodes();
	// remove a traversable graph vertex, if it has two neighboring vertices. Returns the indices
	// of those neighboring vertices, if the vertex has been removed.
	std::set<id_t> mergeNode(id_t);
	// remove this and all neighboring vertices, if they only have two neighboring vertices.
	void mergeNodes(id_t);
	// re-introduce the vertex and the corresponding to it vertex "chain".
	void unfoldNodes(id_t);

	// vector index corresponds to the referenced node, i.e. for each node, input and output connections
	// are to be found in Connections.
	std::vector<Connections> _connections;

	// the chains are merged. For the merged nodes, this saves the endpoints of a chain.
	// vector index = node id.
	std::vector<std::set<id_t>> _connectionRef;
};