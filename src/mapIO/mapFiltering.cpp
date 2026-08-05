#include "mapFiltering.h"
#include <set>
#include <map>

MapInfoTuple dropUntraversableNodes(const Nodes& nodes,
									const Ways& ways,
									const std::vector<Relation>& relations) {
	Nodes resultNodes;
	Ways resultWays;
	std::vector<Relation> resultRelations;

	std::set<id_t> inclNodes;
	std::set<id_t> inclWays;
	auto nodeRefCount = std::vector<std::set<id_t>>(nodes.size());

	for (const auto& way : ways) {
		if (way.refs().empty() || !way.hasTag("highway"))
			continue;
		if (way.hasTag("area") && way.tagValue("area") == "yes")
			continue;

		for (const id_t id : way.refs())
			nodeRefCount[id].insert(way.id());
	}

	for (const auto& way : ways) {
		if (way.refs().empty() || !way.hasTag("highway"))
			continue;
		if (way.hasTag("area") && way.tagValue("area") == "yes")
			continue;

		for (const id_t id : way.refs())
			if (nodeRefCount[id].size() > 1)
			{
				inclWays.insert(way.id());
				resultWays.push_back(way);
				break;
			}
	}

	for (const auto& way : resultWays)
		for (const id_t id : way.refs())
			inclNodes.insert(id);

	for (const auto& node : nodes)
        if (inclNodes.find(node.id()) != inclNodes.end())
			resultNodes.emplace_back(node);

	for (const auto& rel : relations) {
		const auto [from, via, to] = rel.refs();
        if (inclWays.find(from) != inclWays.end() && inclWays.find(to) != inclWays.end() && inclNodes.find(via) != inclNodes.end())
            resultRelations.emplace_back(rel);
	}

	return { resultNodes, resultWays, resultRelations };
}
