#pragma once

#include <vector>
#include <string>
#include <fstream>

class Node;
class Way;
class Relation;
class Bounds;

class xmlWriter {
public:
	void write(const std::string& filename,
			   const std::vector<Node>&,
			   const std::vector<Way>&,
			   const std::vector<Relation>&,
			   const Bounds&) const;

private:
	void writeNode(std::ofstream& file, const Node& node) const;
	void writeWay(std::ofstream& file, const Way& node) const;
	void writeRelation(std::ofstream& file, const Relation& node) const;
};