#pragma once

#include "core.h"
#include <array>
#include <tuple>
#include <vector>

class xmlWriter;
class Node;
class Way;

struct ScaleAreaInformation {
	// create-function, so that no implicit conversion makes troubles
	static ScaleAreaInformation create(double d_scale, double d_x, double d_y);

	size_t scale;
	size_t x;
	size_t y;

	size_t numAreas() const;
	size_t areaIndex() const;

    // returns all areas of all scales, that intersect with the given node.
    static std::vector<ScaleAreaInformation> areaInfos(const Node&);
    // returns all areas of all scales, that intersect with the given way.
    static std::vector<ScaleAreaInformation> areaInfos(const Way&);
	// returns all areas of all scales, that intersect with the given rectangle.
	static std::vector<ScaleAreaInformation> areaInfos(double x1, double y1, double x2, double y2);
};

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

	// scale and grid coordinates for this Bounds instance.
	ScaleAreaInformation scaleAndCoords() const;

private:
	double _minLat = 0.;
	double _maxLat = 0.;
	double _minLon = 0.;
	double _maxLon = 0.;

	friend class xmlWriter;
};
