#include "trafficDummy.h"

#include "trafficObject.h"

TrafficDummy::TrafficDummy(const TrafficObject& object) : _speed(object.speed()), _progress(object.progressOnCurrentSegment())
{}