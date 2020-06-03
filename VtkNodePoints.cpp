#include "VtkNodePoints.h"
#include "VtkNodes.h"
#include "Mesh.h"

void VtkNodePoints::ComputeBounds()
{
	const auto &realNodes = nodes->mesh->nodes;
	if (realNodes.empty()) {
		return;
	}

	std::array<float, 6> floatBounds;
	auto &[xmin, xmax, ymin, ymax, zmin, zmax] = floatBounds;
	xmin = ymin = zmin = std::numeric_limits<float>::max();
	xmax = ymax = zmax = std::numeric_limits<float>::min();
	for (const auto &[nodeNumber, node]: realNodes) {
		xmin = std::min(xmin, node.x);
		xmax = std::max(xmax, node.x);
		ymin = std::min(ymin, node.y);
		ymax = std::max(ymax, node.y);
		zmin = std::min(zmin, node.z);
		zmax = std::max(zmax, node.z);
	}

	for (int i = 0; i < 6; ++i) {
		Bounds[i] = floatBounds[i];
	}
}

VtkNodePoints *VtkNodePoints::New()
{
	return new VtkNodePoints;
}
