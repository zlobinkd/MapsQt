#pragma once

#include <vector>
#include <optional>
#include <string>

class Node;
class Way;
class Relation;
class Bounds;

using XmlOutputTuple = std::tuple<std::vector<Node>, 
								  std::vector<Way>, 
								  std::vector<Relation>,  
								  Bounds>;

std::optional<XmlOutputTuple> parseXML(const std::string& filename);