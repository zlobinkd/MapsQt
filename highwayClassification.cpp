#include "highwayClassification.h"

bool isRoad(std::string routeName) {
    if (routeName == "footway" || routeName == "path" || routeName == "pedestrian"
        || routeName == "steps" || routeName == "elevator" || routeName == "platform"
        || routeName == "bridleway") {
        return false;
    }
    else if (routeName == "motorway" || routeName == "primary") {
        return true;
    }
    else if (routeName == "motorway_link" || routeName == "primary_link") {
        return true;
    }
    else if (routeName == "unclassified" || routeName == "tertiary" || routeName == "secondary"
             || routeName == "secondary_link" || routeName == "tertiary_link" || routeName == "residential") {
        return true;
    }
    else if (routeName == "track" || routeName == "construction" || routeName == "service") {
        return false;
    }
    else if (routeName == "cycleway") {
        return false;
    }
    else if (routeName == "trunk") {
        return true;
    }
    else if (routeName == "trunk_link") {
        return true;
    }
    else if (routeName == "living_street") {
        return true;
    }
    return false;
}

bool isMainRoad(std::string routeName) {
    if (routeName == "footway" || routeName == "path" || routeName == "pedestrian"
        || routeName == "steps" || routeName == "elevator" || routeName == "platform"
        || routeName == "bridleway") {
        return false;
    }
    else if (routeName == "motorway" || routeName == "primary") {
        return true;
    }
    else if (routeName == "motorway_link" || routeName == "primary_link") {
        return false;
    }
    else if (routeName == "unclassified" || routeName == "tertiary" || routeName == "secondary"
             || routeName == "secondary_link" || routeName == "tertiary_link" || routeName == "residential") {
        return false;
    }
    else if (routeName == "track" || routeName == "construction" || routeName == "service") {
        return false;
    }
    else if (routeName == "cycleway") {
        return false;
    }
    else if (routeName == "trunk") {
        return true;
    }
    else if (routeName == "trunk_link") {
        return false;
    }
    else if (routeName == "living_street") {
        return false;
    }
    return false;
}
