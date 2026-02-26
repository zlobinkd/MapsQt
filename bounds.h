#pragma once

#include "core.h"
#include <array>
#include <tuple>
#include <vector>

class xmlWriter;
class Node;
class Way;

// class for boundaries of an aerial image.
// lon - longtitude, lat - latitude.
class Bounds {
public:
	Bounds() {}
	Bounds(double minLat, double maxLat, double minLon, double maxLon);

	// get pixel coords from latitude and longtitude
	std::array<double, 2> localCoords(double lat, double lon) const;
	// get latitude and longtitude from the pixel position
	std::array<double, 2> globalCoords(double x, double y) const;

	double aspectRatio() const;

	// shift current boundaries for x * <vertical image size> vertically
	// and for y * <horizontal image size> horizontally
	void shift(double x, double y);
	// zoom in / zoom out at the point x,y
	void zoomIn(double x, double y);
	void zoomOut(double x, double y);

	// the complete map is divided into different number of areas depending on the zooming scale.
	//
	// SCALE 0 : 1 area
	// +----------------------------------------+
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                   1                    |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// |                                        |
	// +----------------------------------------+
	//
	// SCALE 1 : 4 areas with overlap 50%
	// +--------------------------+
	// |             +------------|-------------+
	// |             |            |             |
	// |        1    |            |             |
	// |             |            |             |
	// |             |            |   2         |
	// |+------------|------------|+            |
	// ||            |+-----------||------------|+
	// ||            ||           ||            ||
	// ||            ||           ||            ||
	// ||            ||           ||            ||
	// +|------------||-----------+|            ||
	//  |            +|------------|------------+|
	//  |        3    |            |  4          |
	//  |             |            |             |
	//  |             |            |             |
	//  |             |            |             |
	//  +-------------|------------+             |
	//                +--------------------------+
	// 
	// SCALE 3 : 16 areas with overlap 50%
	// and so on
	//

	// relative scale and grid coordinates for another Bounds instance.
	std::tuple<size_t, size_t, size_t> scaleAndCoords(const Bounds& other) const;

	// for all scales, returns the pairs of grid coordinates that correspond to the given <coords>.
	// TODO: <coords> is in fact two points (x1, y1, x2, y2) -> convert to a better type.
	// The output has pairs of coordinates, because there could be several grid positions sitting
	// between the left and right input points.
	std::vector<std::array<std::array<size_t, 2>, 2>> scalesAndCoords(const std::array<double, 4>& coords) const;
	// grid coordinates of all scales for a node point.
	std::vector<std::array<std::array<size_t, 2>, 2>> scalesAndCoords(const Node&) const;
	// grid coordinates for a way.
    std::vector<std::array<std::array<size_t, 2>, 2>> scalesAndCoords(const Way&) const;

private:
	double _minLat;
	double _maxLat;
	double _minLon;
	double _maxLon;

	friend class xmlWriter;
};
