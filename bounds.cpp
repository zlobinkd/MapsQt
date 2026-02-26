#include "bounds.h"
#include "node.h"
#include "way.h"
#include "mapData.h"
#include <cmath>
#include <algorithm>

#define M_PI           3.14159265358979323846

Bounds::Bounds(double minLat, double maxLat, double minLon, double maxLon)
	: _minLat(minLat), 
	  _maxLat(maxLat), 
	  _minLon(minLon), 
	  _maxLon(maxLon) 
{}

// relative image coordinates in [0,1] range
// approx, no spherical stuff
std::array<double, 2> Bounds::localCoords(double lat, double lon) const {
	if (_minLat == _maxLat)
		return { 0., 0. };

	// invert the latitude: image origin is the upper left corner
    const double x = (_maxLat - lat) / (_maxLat - _minLat);
    const double y = (lon - _minLon) / (_maxLon - _minLon);
    return { x, y };
}

// approx
std::array<double, 2> Bounds::globalCoords(double x, double y) const {
	if (_minLat == _maxLat)
		return { 0., 0. };

    const double lat = _maxLat - x * (_maxLat - _minLat);
    const double lon = _minLon + y * (_maxLon - _minLon);
	return { lat, lon };
}

double Bounds::aspectRatio() const {
	return (_maxLon - _minLon) / (_maxLat - _minLat) * cos(_minLat * M_PI / 180.);
}

void Bounds::zoomIn(double x, double y) {
    const double dx = _maxLat - _minLat;
    const double dy = _maxLon - _minLon;
    _minLat += 1. / 3. * x * dx;
    _maxLat -= 1. / 3. * (1. - x) * dx;
    _minLon += 1. / 3. * y * dy;
    _maxLon -= 1. / 3. * (1. - y) * dy;
}

void Bounds::zoomOut(double x, double y) {
    const double dx = _maxLat - _minLat;
    const double dy = _maxLon - _minLon;
    _minLat -= 0.5 * x * dx;
    _maxLat += 0.5 * (1. - x) * dx;
    _minLon -= 0.5 * y * dy;
    _maxLon += 0.5 * (1. - y) * dy;
}

void Bounds::shift(double x, double y) {
    const double dx = _maxLat - _minLat;
    const double dy = _maxLon - _minLon;
    _minLat += x * dx;
    _maxLat += x * dx;
    _minLon += y * dy;
    _maxLon += y * dy;
}

// ??
std::tuple<size_t, size_t, size_t> Bounds::scaleAndCoords(const Bounds& other) const {
    const double ratioLat = (_maxLat - _minLat) / (other._maxLat - other._minLat);
    const double ratioLon = (_maxLon - _minLon) / (other._maxLon - other._minLon);
    const double ratio = std::min(ratioLat, ratioLon);
	const size_t scale = std::clamp(std::log2(ratio), 0., double(SCALES_NUM - 1));
	const size_t areasNum = 1 << scale;
	const double posX = (_maxLat - (other._maxLat + other._minLat) / 2.) / (_maxLat - _minLat) * (areasNum + 1) - 0.5;
	const size_t x = std::clamp(posX, 0., double(areasNum - 1));

	const double posY = ((other._maxLon + other._minLon) / 2. - _minLon) / (_maxLon - _minLon) * (areasNum + 1) - 0.5;
	const size_t y = std::clamp(posY, 0., double(areasNum - 1));

	return { scale, x, y };
}

// ???
std::vector<std::array<std::array<size_t, 2>, 2>> Bounds::scalesAndCoords(const std::array<double, 4>& coords) const {
	std::vector<std::array<std::array<size_t, 2>, 2>> res;
    const auto [x1, y1, x2, y2] = coords;
	for (size_t scale = 0; scale < SCALES_NUM; scale++) {
		const size_t numAreas = 1 << scale;
		const size_t i1 = std::min(numAreas - 1, size_t(std::max(0., x1 * (numAreas + 1) - 1.)));
		const size_t i2 = std::min(numAreas - 1, size_t(std::max(0., x2 * (numAreas + 1))));
		const size_t j1 = std::min(numAreas - 1, size_t(std::max(0., y1 * (numAreas + 1) - 1.)));
		const size_t j2 = std::min(numAreas - 1, size_t(std::max(0., y2 * (numAreas + 1))));
		res.push_back({ std::array<size_t, 2>{i1, i2}, std::array<size_t, 2>{j1, j2} });
	}
	return res;
}

std::vector<std::array<std::array<size_t, 2>, 2>> Bounds::scalesAndCoords(const Node& node) const {
	const auto [x, y] = node.localCoords(*this);
	return scalesAndCoords({ x, y, x, y });
}

std::vector<std::array<std::array<size_t, 2>, 2>> Bounds::scalesAndCoords(const Way& way) const {
	double x1 = 100.;
	double x2 = -100.;
	double y1 = 100.;
	double y2 = -100.;
	for (const id_t ref : way.refs()) {
		const auto [x, y] = MapData::instance().nodes()[ref].localCoords(*this);
		if (x < x1)
			x1 = x;

		if (x > x2)
			x2 = x;

		if (y < y1)
			y1 = y;

		if (y > y2)
			y2 = y;
	}

	return scalesAndCoords({ x1, y1, x2, y2 });
}
