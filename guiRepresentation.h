#pragma once

#include "node.h"
#include "way.h"
#include "bounds.h"

#include "QVector"
#include "QPen"
#include "QLineF"

#include "hash.h"

// map representation, optimized for GUI.
class GuiRepresentation {
public:
	GuiRepresentation();

	// ways from the grid position according to current image boundaries
    const QHash<QPair<QColor, int>, QVector<QLineF>>& waysToVisualize(const Bounds&) const;
	// closest map entry for the given arbitrary position
	id_t closestPoint(double lat, double lon, const Bounds&) const;

    constexpr static int ImageSize = 25000;

private:
	// grid position entry.
	class Area {
	public:
		Area() = default;

		void insertNode(id_t);
		void insertWay(id_t);

		const std::vector<id_t>& nodes() const;
        const QHash<QPair<QColor, int>, QVector<QLineF>>& ways() const;

	private:
		std::vector<id_t> _nodes;
        QHash<QPair<QColor, int>, QVector<QLineF>> _ways;
	};

	// grid.
	using MapImpl = std::vector<std::vector<Area>>;

	MapImpl _map;
};
