#include "bounds.h"
#include "node.h"
#include "way.h"
#include "mapData.h"
#include <cmath>
#include <algorithm>

#define M_PI           3.14159265358979323846

ScaleAreaInformation ScaleAreaInformation::create(const double d_scale, const double d_x, const double d_y) {
	const size_t scale = std::clamp(std::log2(d_scale), 0., double(SCALES_NUM - 1));
	const size_t areasNum = 1 << scale;

	const double posX = d_x * (areasNum + 1) - 0.5;
    const size_t x = std::clamp(posX, 0., double(areasNum - 1));

	const double posY = d_y * (areasNum + 1) - 0.5;
    const size_t y = std::clamp(posY, 0., double(areasNum - 1));
	return { scale, x, y };
}

size_t ScaleAreaInformation::numAreas() const {
	return 1 << scale;
}

size_t ScaleAreaInformation::areaIndex() const {
	return numAreas() * x + y;
}

std::vector<ScaleAreaInformation> ScaleAreaInformation::areaInfos(const double x1, const double y1, const double x2, const double y2) {
	std::vector<ScaleAreaInformation> res;
	for (size_t scale = 0; scale < SCALES_NUM; scale++) {
		const size_t numAreas = 1 << scale;
		const size_t i1 = std::min(numAreas - 1, size_t(std::max(0., x1 * (numAreas + 1) - 1.)));
		const size_t i2 = std::min(numAreas - 1, size_t(std::max(0., x2 * (numAreas + 1))));
		const size_t j1 = std::min(numAreas - 1, size_t(std::max(0., y1 * (numAreas + 1) - 1.)));
		const size_t j2 = std::min(numAreas - 1, size_t(std::max(0., y2 * (numAreas + 1))));
		for (size_t i = i1; i <= i2; i++)
			for (size_t j = j1; j <= j2; j++)
				res.push_back({ scale, i, j });
	}
	return res;
}

std::vector<ScaleAreaInformation> ScaleAreaInformation::areaInfos(const Node& node) {
    const auto [x, y] = node.localCoords(MapData::instance().bounds());
    return areaInfos(x, y, x, y);
}

std::vector<ScaleAreaInformation> ScaleAreaInformation::areaInfos(const Way& way) {
    double x1 = 100.;
    double x2 = -100.;
    double y1 = 100.;
    double y2 = -100.;
    for (const id_t ref : way.refs()) {
        const auto [x, y] = MapData::instance().nodes()[ref].localCoords(MapData::instance().bounds());
        if (x < x1)
            x1 = x;

        if (x > x2)
            x2 = x;

        if (y < y1)
            y1 = y;

        if (y > y2)
            y2 = y;
    }

    return areaInfos(x1, y1, x2, y2);
}

Bounds::Bounds(const double minLat, const double maxLat, const double minLon, const double maxLon)
	: _minLat(minLat), 
	  _maxLat(maxLat), 
	  _minLon(minLon), 
	  _maxLon(maxLon) 
{}

// relative image coordinates in [0,1] range
// approx, no spherical stuff
std::array<double, 2> Bounds::localCoords(const double lat, const double lon) const {
	if (_minLat == _maxLat)
		return { 0., 0. };

	// invert the latitude: image origin is the upper left corner
	const double x = (_maxLat - lat) / (_maxLat - _minLat);
	const double y = (lon - _minLon) / (_maxLon - _minLon);
	return { x, y };
}

// approx
std::array<double, 2> Bounds::globalCoords(const double x, const double y) const {
	if (_minLat == _maxLat)
		return { 0., 0. };

	const double lat = _maxLat - x * (_maxLat - _minLat);
	const double lon = _minLon + y * (_maxLon - _minLon);
	return { lat, lon };
}

double Bounds::aspectRatio() const {
	return (_maxLon - _minLon) / (_maxLat - _minLat) * cos(_minLat * M_PI / 180.);
}

void Bounds::zoomIn(const double x, const double y) {
	const double dx = _maxLat - _minLat;
	const double dy = _maxLon - _minLon;
	_minLat += 1. / 3. * x * dx;
	_maxLat -= 1. / 3. * (1. - x) * dx;
	_minLon += 1. / 3. * y * dy;
	_maxLon -= 1. / 3. * (1. - y) * dy;
}

void Bounds::zoomOut(const double x, const double y) {
	const double dx = _maxLat - _minLat;
	const double dy = _maxLon - _minLon;
	_minLat -= 0.5 * x * dx;
	_maxLat += 0.5 * (1. - x) * dx;
	_minLon -= 0.5 * y * dy;
	_maxLon += 0.5 * (1. - y) * dy;
}

void Bounds::shift(const double x, const double y) {
	const double dx = _maxLat - _minLat;
	const double dy = _maxLon - _minLon;
	_minLat += x * dx;
	_maxLat += x * dx;
	_minLon += y * dy;
	_maxLon += y * dy;
}

ScaleAreaInformation Bounds::scaleAndCoords() const {
	const double globalMinLat = MapData::instance().bounds()._minLat;
	const double globalMaxLat = MapData::instance().bounds()._maxLat;
	const double globalMinLon = MapData::instance().bounds()._minLon;
	const double globalMaxLon = MapData::instance().bounds()._maxLon;

	const double globalDLat = globalMaxLat - globalMinLat;
	const double globalDLon = globalMaxLon - globalMinLon;

	const double dLat = _maxLat - _minLat;
	const double dLon = _maxLon - _minLon;

	const double ratioLat = globalDLat / dLat;
	const double ratioLon = globalDLon / dLon;
    const double ratio = std::min(ratioLat, ratioLon);

	const double posX = (globalMaxLat - (_maxLat + _minLat) / 2.) / globalDLat;
	const double posY = ((_maxLon + _minLon) / 2. - globalMinLon) / globalDLon;

    return ScaleAreaInformation::create(ratio, posX, posY);
}
