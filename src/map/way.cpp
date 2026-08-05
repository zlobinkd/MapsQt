#include "way.h"
#include "bounds.h"

#include <ranges>

Way::Way(id_t id, const std::vector<id_t>& nodeIds, const Tags& tags)
	: _id(id), _nodeIds(nodeIds), _tags(tags) 
{}

id_t Way::id() const {
	return _id;
}

void Way::setId(id_t id) {
	_id = id;
}

bool Way::hasTag(const std::string& tag) const {
	for (const auto& [key, _] : _tags)
		if (key == tag)
			return true;

	return false;
}

std::string Way::tagValue(const std::string& tag) const {
	for (const auto& [key, value] : _tags)
		if (key == tag)
			return value;

	return "";
}

const std::vector<id_t>& Way::refs() const {
	return _nodeIds;
}

double Way::speedLimit() const {
	if (!hasTag("maxspeed"))
	{
		if (tagValue("highway") == "residential")
			return 30. / 3.6;
		if (tagValue("highway") == "secondary_link")
			return 60. / 3.6;
		if (tagValue("highway") == "primary_link")
			return 60. / 3.6;
		if (tagValue("highway") == "secondary")
			return 60. / 3.6;
		if (tagValue("highway") == "primary")
			return 70. / 3.6;
		if (tagValue("highway") == "tertiary")
			return 30. / 3.6;
		return 5. / 3.6;
	}

	const std::string s = tagValue("maxspeed");
	if (s == "walk")
		return 5. / 3.6;

	if (s == "none")
		return 130. / 3.6;

	if (s == "RU:urban")
		return 60. / 3.6;

	if (s == "RU:rural")
		return 90. / 3.6;

	if (s == "RU:motorway")
		return 110. / 3.6;

	if (s == "RU:living_street")
		return 15. / 3.6;

	return std::stod(s) / 3.6;
}
